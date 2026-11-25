#include "global.h"
#include "gba/m4a_internal.h"
#include "sound.h"
#include "battle.h"
#include "m4a.h"
#include "main.h"
#include "pokemon.h"
#include "overworld.h"
#include "constants/songs.h"
#include "task.h"
#include "data/night_music.h"

struct Fanfare
{
    u16 songNum;
    u16 duration;
};

EWRAM_DATA struct MusicPlayerInfo *gMPlay_PokemonCry = NULL;
EWRAM_DATA u8 gPokemonCryBGMDuckingCounter = 0;

static u16 sCurrentMapMusic;
static u16 sPlayingMapMusic; // what sCurrentMapMusic was started as: itself, or its night version
static u16 sNextMapMusic;
static u8 sMapMusicState;
static u8 sMapMusicFadeInSpeed;
static u16 sFanfareCounter;

#define NUM_TRACKS_BGM 10 // (sound/music_player_table.inc)

// Commands of a song's tracks (include/MPlayDef.s)
#define CMD_WAIT_LAST 0xB0 // W96
#define CMD_FINE      0xB1
#define CMD_GOTO      0xB2
#define CMD_MEMACC    0xB9
#define CMD_TEMPO     0xBB
#define CMD_VOICE     0xBD
#define CMD_PORT      0xCC
#define CMD_TIE       0xCF // and the notes after it

// How much of the other version of the map music is run in a frame as it's swapped in: this
// many commands at most (a tenth of a frame; the longest songs then take two seconds), and
// no more than fit in about half of the time the frame has left, so no frame is dropped for
// it. A frame with no time for more than the song playing gets through itself is left alone;
// after a few of them in a row, that much is run anyway, or the song is never caught up.
#define SWAP_COMMANDS_PER_FRAME    128
#define SWAP_COMMANDS_MIN          24
#define SWAP_COMMANDS_PER_SCANLINE 3 // (one takes about 200 of a scanline's 1232 cycles)
#define SWAP_BUSY_FRAMES           8
#define SCANLINES_PER_FRAME        228

enum
{
    SWAP_NONE,
    SWAP_FIND_LOOP, // running the first track alone, to learn where the song loops
    SWAP_SEEK,      // running every track up to where the song playing is
    SWAP_FAILED,    // the two aren't the same song: one has to be faded into the other
};

static EWRAM_DATA struct
{
    u8 state;
    u8 trackCount;             // tracks being run
    u8 loops;                  // times the first track has looped
    u8 busyFrames;             // frames in a row that had no time for this
    u16 songNum;               // the version being swapped in
    struct SongHeader *playing; // the version it takes the place of
    u32 loopEnd;               // the tick the song first loops on
    u32 loopLength;            // 0 if it doesn't loop
    u32 clockOffset;           // the whole loops of the song playing that aren't run
    u32 tempoClock;            // the tick of the last tempo command run, and
    u16 tempoD;                // its tempo
    u32 clocks[NUM_TRACKS_BGM]; // ticks each spare track has run
    struct MusicPlayerInfo info;
    struct MusicPlayerTrack tracks[NUM_TRACKS_BGM];
} sMusicSwap = {0};

COMMON_DATA bool8 gDisableMusic = 0;

extern struct ToneData gCryTable[];
extern struct ToneData gCryTable_Reverse[];

static void Task_Fanfare(u8 taskId);
static void CreateFanfareTask(void);
static void Task_DuckBGMForPokemonCry(u8 taskId);
static void RestoreBGMVolumeAfterPokemonCry(void);
static void UpdateMusicSwap(void);

// The 1st argument in the table is the length of the fanfare, measured in frames. This is calculated by taking the duration of the midi file, multiplying by 59.72750056960583, and rounding up to the next nearest integer.
static const struct Fanfare sFanfares[] = {
    [FANFARE_LEVEL_UP]            = { MUS_LEVEL_UP,             80 },
    [FANFARE_OBTAIN_ITEM]         = { MUS_OBTAIN_ITEM,         160 },
    [FANFARE_EVOLVED]             = { MUS_EVOLVED,             220 },
    [FANFARE_OBTAIN_TMHM]         = { MUS_OBTAIN_TMHM,         220 },
    [FANFARE_HEAL]                = { MUS_HEAL,                160 },
    [FANFARE_OBTAIN_BADGE]        = { MUS_OBTAIN_BADGE,        340 },
    [FANFARE_MOVE_DELETED]        = { MUS_MOVE_DELETED,        180 },
    [FANFARE_OBTAIN_BERRY]        = { MUS_OBTAIN_BERRY,        120 },
    [FANFARE_AWAKEN_LEGEND]       = { MUS_AWAKEN_LEGEND,       710 },
    [FANFARE_SLOTS_JACKPOT]       = { MUS_SLOTS_JACKPOT,       250 },
    [FANFARE_SLOTS_WIN]           = { MUS_SLOTS_WIN,           150 },
    [FANFARE_TOO_BAD]             = { MUS_TOO_BAD,             160 },
    [FANFARE_RG_POKE_FLUTE]       = { MUS_RG_POKE_FLUTE,       450 },
    [FANFARE_RG_OBTAIN_KEY_ITEM]  = { MUS_RG_OBTAIN_KEY_ITEM,  170 },
    [FANFARE_RG_DEX_RATING]       = { MUS_RG_DEX_RATING,       196 },
    [FANFARE_OBTAIN_B_POINTS]     = { MUS_OBTAIN_B_POINTS,     313 },
    [FANFARE_OBTAIN_SYMBOL]       = { MUS_OBTAIN_SYMBOL,       318 },
    [FANFARE_REGISTER_MATCH_CALL] = { MUS_REGISTER_MATCH_CALL, 135 },
    [FANFARE_RG_PHOTO]            = { MUS_RG_PHOTO,            75  },
};

void InitMapMusic(void)
{
    gDisableMusic = FALSE;
    ResetMapMusic();
}

static u8 sBGMFadeProtection;

static u16 GetNightMusic(u16 songNum)
{
    if (songNum >= NIGHT_MUSIC_FIRST && songNum <= NIGHT_MUSIC_LAST && sNightMusic[songNum - NIGHT_MUSIC_FIRST] != 0)
        return sNightMusic[songNum - NIGHT_MUSIC_FIRST];
    return songNum;
}

// At night, the songs of Hoenn's and Kanto's maps and battles play in their night versions
// (made by tech/night_music.py). Map music is still kept by the day song, which is what
// GetCurrentMapMusic returns and is compared with; it's only turned into the night one here,
// as it starts playing. A song started with PlayBGM stays as it is.
u16 GetTimeOfDayMusic(u16 songNum)
{
    if (GetNightMusic(songNum) != songNum && GetCurrentTimeOfDay() == TIME_OF_DAY_NIGHT)
        return GetNightMusic(songNum);
    return songNum;
}

static void PlayMapBGM(u16 songNum)
{
    sPlayingMapMusic = GetTimeOfDayMusic(songNum);
    PlayBGM(sPlayingMapMusic);
}

// When night falls or ends, the map music carries on as its other version from where it is,
// with no break.
//
// A night song is its day song note for note (tech/night_music.py changes the tempo, the
// vibrato and some voices), so both are at the same place after the same number of ticks.
// Their data isn't laid out alike, though, so that place has to be found: the other version
// is run from its start on spare tracks, its notes left unplayed, until it has run as many
// ticks as the song playing has. Its tracks then take the place of the ones playing, which
// keep the notes that are sounding. It's run a little each frame (UpdateMusicSwap), while
// the song plays on.

// Sets the spare tracks up at the start of the song, as MPlayStart and MPlayMain do.
static void StartMusicSwapRun(u8 state, u8 trackCount)
{
    struct SongHeader *header = gSongTable[sMusicSwap.songNum].header;
    struct MusicPlayerTrack *track = sMusicSwap.tracks;
    u32 i;

    memset(&sMusicSwap.info, 0, sizeof(sMusicSwap.info));
    sMusicSwap.info.songHeader = header;
    sMusicSwap.info.tone = header->tone;
    for (i = 0; i < trackCount; i++, track++)
    {
        memset(track, 0, sizeof(*track));
        track->flags = MPT_FLG_EXIST;
        track->bendRange = 2;
        track->volX = 64;
        track->lfoSpeed = 22;
        track->tone.type = 1;
        track->cmdPtr = header->part[i];
        sMusicSwap.clocks[i] = 0;
    }
    sMusicSwap.state = state;
    sMusicSwap.trackCount = trackCount;
    sMusicSwap.tempoD = 150;
    sMusicSwap.tempoClock = 0;
}

// Runs a spare track on until it has run `target` ticks, or the budget of commands is spent
// (the tracks don't depend on each other, so each is run on its own). Returns what's left of
// the budget. The commands are read the way MPlayMain reads them.
static s32 RunMusicSwapTrack(u32 trackId, u32 target, s32 budget)
{
    struct MusicPlayerTrack track; // (worked on in the stack's faster RAM)
    u32 clock = sMusicSwap.clocks[trackId];
    u32 cmd, ticks;
    u8 *ptr;

    if (!(sMusicSwap.tracks[trackId].flags & MPT_FLG_EXIST))
    {
        sMusicSwap.clocks[trackId] = target;
        return budget;
    }

    track = sMusicSwap.tracks[trackId];
    while (clock < target && budget > 0)
    {
        if (track.wait != 0)
        {
            ticks = target - clock;
            if (ticks > track.wait)
                ticks = track.wait;
            track.wait -= ticks;
            clock += ticks;
            continue;
        }

        do
        {
            budget--;
            ptr = track.cmdPtr;
            cmd = *ptr;
            if (cmd < 0x80)
            {
                cmd = track.runningStatus;
            }
            else
            {
                ptr++;
                if (cmd >= CMD_VOICE)
                    track.runningStatus = cmd;
            }

            if (cmd >= CMD_TIE)
            {
                // A note isn't played. Its key and velocity are also those of the notes
                // after it that give none, so they are kept (as ply_note does)
                if (*ptr < 0x80)
                {
                    track.key = *ptr++;
                    if (*ptr < 0x80)
                    {
                        track.velocity = *ptr++;
                        if (*ptr < 0x80)
                            ptr++;
                    }
                }
                track.cmdPtr = ptr;
            }
            else if (cmd <= CMD_WAIT_LAST)
            {
                track.wait = gClockTable[cmd - 0x80];
                track.cmdPtr = ptr;
            }
            else if (cmd == CMD_TEMPO)
            {
                // (the song's, not the track's: the last one in the song's order counts)
                if (clock >= sMusicSwap.tempoClock)
                {
                    sMusicSwap.tempoClock = clock;
                    sMusicSwap.tempoD = *ptr * 2;
                }
                track.cmdPtr = ptr + 1;
            }
            else if (cmd == CMD_PORT)
            {
                // (a write to a sound register: not this song's to make yet)
                track.cmdPtr = ptr + 2;
            }
            else if (cmd == CMD_MEMACC)
            {
                // (it reads and writes what the song playing does)
                sMusicSwap.state = SWAP_FAILED;
                track.flags = 0;
            }
            else
            {
                if (cmd == CMD_GOTO && sMusicSwap.state == SWAP_FIND_LOOP)
                {
                    if (sMusicSwap.loops++ == 0)
                    {
                        sMusicSwap.loopEnd = clock;
                    }
                    else
                    {
                        sMusicSwap.loopLength = clock - sMusicSwap.loopEnd;
                        target = 0;
                    }
                }
                // The spare tracks have no notes playing, so the other commands only set
                // the track up
                track.cmdPtr = ptr;
                sMusicSwap.info.cmd = cmd - CMD_FINE;
                gMPlayJumpTable[cmd - CMD_FINE](&sMusicSwap.info, &track);
            }
        } while (track.wait == 0 && track.flags != 0);

        if (track.flags == 0)
        {
            clock = target;
            break;
        }
        track.wait--;
        clock++;
    }
    sMusicSwap.tracks[trackId] = track;
    sMusicSwap.clocks[trackId] = clock;
    return budget;
}

// The spare tracks are where the song playing is: they become its tracks.
// FALSE if the song got a tick further in the meantime.
static bool32 FinishMusicSwap(u32 clock)
{
    struct MusicPlayerInfo *mplayInfo = &gMPlayInfo_BGM;
    struct SongHeader *header = sMusicSwap.info.songHeader;
    struct MusicPlayerTrack *track = mplayInfo->tracks;
    struct MusicPlayerTrack *spare = sMusicSwap.tracks;
    u32 i;

    if (mplayInfo->ident != ID_NUMBER)
        return FALSE;
    mplayInfo->ident++; // (keeps MPlayMain, which runs in the VBlank interrupt, off the tracks)
    if (mplayInfo->clock != clock)
    {
        mplayInfo->ident = ID_NUMBER;
        return FALSE;
    }

    for (i = 0; i < sMusicSwap.trackCount; i++, track++, spare++)
    {
        if (!(spare->flags & MPT_FLG_EXIST))
        {
            TrackStop(mplayInfo, track);
            track->flags = 0;
            continue;
        }
        // The track keeps its notes that are sounding, what was set from outside the song
        // (the volume of a fade or of a cry playing over it), and its vibrato's phase
        spare->chan = track->chan;
        spare->volX = track->volX;
        spare->panX = track->panX;
        spare->pitX = track->pitX;
        spare->keyShiftX = track->keyShiftX;
        spare->lfoDelayC = track->lfoDelayC;
        if (spare->mod != 0 && spare->lfoSpeed != 0)
        {
            spare->lfoSpeedC = track->lfoSpeedC;
            spare->modM = track->modM;
        }
        spare->flags |= MPT_FLG_VOLCHG | MPT_FLG_PITCHG;
        *track = *spare;
    }
    mplayInfo->songHeader = header;
    mplayInfo->tone = header->tone;
    mplayInfo->priority = header->priority;
    mplayInfo->tempoD = sMusicSwap.tempoD;
    mplayInfo->tempoI = (mplayInfo->tempoD * mplayInfo->tempoU) >> 8;
    mplayInfo->ident = ID_NUMBER;

    sPlayingMapMusic = sMusicSwap.songNum;
    sMusicSwap.state = SWAP_NONE;
    return TRUE;
}

// The overworld calls this as it keeps track of the time of day.
void UpdateMapMusicForTimeOfDay(void)
{
    u16 songNum = sCurrentMapMusic;
    u16 version;
    struct SongHeader *header;

    if (sMapMusicState != 2 || songNum == 0)
        return;
    version = GetTimeOfDayMusic(songNum);
    if (sPlayingMapMusic == version)
    {
        sMusicSwap.state = SWAP_NONE;
        return;
    }
    // (not if something else was started over the map music, or a fanfare has it paused)
    if (gMPlayInfo_BGM.songHeader != gSongTable[sPlayingMapMusic].header)
        return;
    if (IsBGMPausedOrStopped() || !IsFanfareTaskInactive())
        return;
    if (sMusicSwap.state != SWAP_NONE && sMusicSwap.songNum == version)
        return;

    header = gSongTable[version].header;
    if (header->trackCount != gMPlayInfo_BGM.songHeader->trackCount || header->trackCount > NUM_TRACKS_BGM)
    {
        // Not the same song: it can only be faded into
        FadeOutAndFadeInNewMapMusic(songNum, 8, 8);
        return;
    }
    sMusicSwap.songNum = version;
    sMusicSwap.playing = gMPlayInfo_BGM.songHeader;
    sMusicSwap.loops = 0;
    sMusicSwap.loopLength = 0;
    sMusicSwap.busyFrames = 0;
    StartMusicSwapRun(SWAP_FIND_LOOP, 1);
}

static void UpdateMusicSwap(void)
{
    struct MusicPlayerInfo *mplayInfo = &gMPlayInfo_BGM;
    s32 budget;
    u32 i, clock, target, line;
    bool32 ended;

    if (sMusicSwap.state == SWAP_NONE)
        return;
    // Dropped if the song is no longer playing as the map music. If it is again later,
    // UpdateMapMusicForTimeOfDay starts over.
    if (sMapMusicState != 2 || mplayInfo->songHeader != sMusicSwap.playing || IsBGMPausedOrStopped())
    {
        sMusicSwap.state = SWAP_NONE;
        return;
    }

    // This is the last thing the main loop does in a frame: the time until the next VBlank
    // is to spare.
    line = REG_VCOUNT;
    budget = ((line < DISPLAY_HEIGHT ? DISPLAY_HEIGHT : DISPLAY_HEIGHT + SCANLINES_PER_FRAME) - line) * SWAP_COMMANDS_PER_SCANLINE;
    if (budget > SWAP_COMMANDS_PER_FRAME)
        budget = SWAP_COMMANDS_PER_FRAME;
    if (budget >= SWAP_COMMANDS_MIN)
    {
        sMusicSwap.busyFrames = 0;
    }
    else
    {
        if (sMusicSwap.busyFrames < SWAP_BUSY_FRAMES)
        {
            sMusicSwap.busyFrames++;
            return;
        }
        budget = SWAP_COMMANDS_MIN;
    }

    while (budget > 0)
    {
        clock = mplayInfo->clock;
        if (sMusicSwap.state == SWAP_FIND_LOOP)
        {
            budget = RunMusicSwapTrack(0, 0xFFFFFFFF, budget);
            if (sMusicSwap.state == SWAP_FIND_LOOP
             && (sMusicSwap.loops == 2 || !(sMusicSwap.tracks[0].flags & MPT_FLG_EXIST)))
            {
                // The song playing may be many loops in. It's as it was at the same place of
                // its second time through the loop: the loops since are left out.
                sMusicSwap.clockOffset = 0;
                if (sMusicSwap.loops == 2 && sMusicSwap.loopLength != 0 && clock > sMusicSwap.loopEnd)
                    sMusicSwap.clockOffset = (clock - sMusicSwap.loopEnd - 1) / sMusicSwap.loopLength * sMusicSwap.loopLength;
                StartMusicSwapRun(SWAP_SEEK, sMusicSwap.info.songHeader->trackCount);
            }
        }
        else
        {
            target = clock - sMusicSwap.clockOffset;
            ended = TRUE;
            for (i = 0; i < sMusicSwap.trackCount && sMusicSwap.state == SWAP_SEEK; i++)
            {
                if (clock < sMusicSwap.clockOffset || sMusicSwap.clocks[i] > target)
                {
                    // The song was started again
                    sMusicSwap.state = SWAP_NONE;
                    return;
                }
                if (sMusicSwap.clocks[i] < target)
                {
                    budget = RunMusicSwapTrack(i, target, budget);
                    if (sMusicSwap.clocks[i] < target)
                        break;
                }
                if (sMusicSwap.tracks[i].flags & MPT_FLG_EXIST)
                    ended = FALSE;
            }
            if (i == sMusicSwap.trackCount && sMusicSwap.state == SWAP_SEEK)
            {
                if (ended)
                    sMusicSwap.state = SWAP_FAILED; // it ended where the song playing hasn't
                else if (FinishMusicSwap(clock))
                    return;
                else
                    budget--;
            }
        }

        if (sMusicSwap.state == SWAP_FAILED)
        {
            // Not the same song after all: it can only be faded into
            sMusicSwap.state = SWAP_NONE;
            FadeOutAndFadeInNewMapMusic(sCurrentMapMusic, 8, 8);
            return;
        }
    }
}

void MapMusicMain(void)
{
    UpdateMusicSwap();

    // After PlayBGM, protect against spurious fadeOI writes for a few frames.
    // Something during map loading corrupts fadeOI, causing music to fade
    // immediately after starting (e.g. Viridian City).
    if (sBGMFadeProtection > 0)
    {
        sBGMFadeProtection--;
        gMPlayInfo_BGM.fadeOI = 0;
    }

    switch (sMapMusicState)
    {
    case 0:
        break;
    case 1:
        sMapMusicState = 2;
        PlayMapBGM(sCurrentMapMusic);
        sBGMFadeProtection = 10;
        break;
    case 2:
    case 3:
    case 4:
        break;
    case 5:
        if (IsBGMStopped())
        {
            sNextMapMusic = 0;
            sMapMusicState = 0;
        }
        break;
    case 6:
        if (IsBGMStopped() && IsFanfareTaskInactive())
        {
            sCurrentMapMusic = sNextMapMusic;
            sNextMapMusic = 0;
            sMapMusicState = 2;
            PlayMapBGM(sCurrentMapMusic);
            sBGMFadeProtection = 10;
        }
        break;
    case 7:
        if (IsBGMStopped() && IsFanfareTaskInactive())
        {
            sPlayingMapMusic = GetTimeOfDayMusic(sNextMapMusic);
            FadeInNewBGM(sPlayingMapMusic, sMapMusicFadeInSpeed);
            sCurrentMapMusic = sNextMapMusic;
            sNextMapMusic = 0;
            sMapMusicState = 2;
            sMapMusicFadeInSpeed = 0;
        }
        break;
    }
}

void ResetMapMusic(void)
{
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 0;
    sMapMusicFadeInSpeed = 0;
}

u16 GetCurrentMapMusic(void)
{
    return sCurrentMapMusic;
}

void PlayNewMapMusic(u16 songNum)
{
    sCurrentMapMusic = songNum;
    sNextMapMusic = 0;
    sMapMusicState = 1;
}

void PlayNewMapMusicImmediate(u16 songNum)
{
    sCurrentMapMusic = songNum;
    sNextMapMusic = 0;
    sMapMusicState = 2;
    PlayMapBGM(songNum);
    sBGMFadeProtection = 10;
}

void StopMapMusic(void)
{
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 1;
}

void FadeOutMapMusic(u8 speed)
{
    sBGMFadeProtection = 0;
    if (IsNotWaitingForBGMStop())
        FadeOutBGM(speed);
    sCurrentMapMusic = 0;
    sNextMapMusic = 0;
    sMapMusicState = 5;
}

void FadeOutAndPlayNewMapMusic(u16 songNum, u8 speed)
{
    FadeOutMapMusic(speed);
    sCurrentMapMusic = 0;
    sNextMapMusic = songNum;
    sMapMusicState = 6;
}

void FadeOutAndFadeInNewMapMusic(u16 songNum, u8 fadeOutSpeed, u8 fadeInSpeed)
{
    FadeOutMapMusic(fadeOutSpeed);
    sCurrentMapMusic = 0;
    sNextMapMusic = songNum;
    sMapMusicState = 7;
    sMapMusicFadeInSpeed = fadeInSpeed;
}

static void UNUSED FadeInNewMapMusic(u16 songNum, u8 speed)
{
    sPlayingMapMusic = GetTimeOfDayMusic(songNum);
    FadeInNewBGM(sPlayingMapMusic, speed);
    sCurrentMapMusic = songNum;
    sNextMapMusic = 0;
    sMapMusicState = 2;
    sMapMusicFadeInSpeed = 0;
}

bool8 IsNotWaitingForBGMStop(void)
{
    if (sMapMusicState == 6)
        return FALSE;
    if (sMapMusicState == 5)
        return FALSE;
    if (sMapMusicState == 7)
        return FALSE;
    return TRUE;
}

void PlayFanfareByFanfareNum(u8 fanfareNum)
{
    u16 songNum;
    m4aMPlayStop(&gMPlayInfo_BGM);
    songNum = sFanfares[fanfareNum].songNum;
    sFanfareCounter = sFanfares[fanfareNum].duration;
    m4aSongNumStart(songNum);
}

bool8 WaitFanfare(bool8 stop)
{
    if (sFanfareCounter)
    {
        sFanfareCounter--;
        return FALSE;
    }
    else
    {
        if (!stop)
            m4aMPlayContinue(&gMPlayInfo_BGM);
        else
            m4aSongNumStart(MUS_DUMMY);

        return TRUE;
    }
}

// Unused
void StopFanfareByFanfareNum(u8 fanfareNum)
{
    m4aSongNumStop(sFanfares[fanfareNum].songNum);
}

void PlayFanfare(u16 songNum)
{
    s32 i;
    for (i = 0; (u32)i < ARRAY_COUNT(sFanfares); i++)
    {
        if (sFanfares[i].songNum == songNum)
        {
            PlayFanfareByFanfareNum(i);
            CreateFanfareTask();
            return;
        }
    }

    // songNum is not in sFanfares
    // Play first fanfare in table instead
    PlayFanfareByFanfareNum(0);
    CreateFanfareTask();
}

bool8 IsFanfareTaskInactive(void)
{
    if (FuncIsActiveTask(Task_Fanfare) == TRUE)
        return FALSE;
    return TRUE;
}

static void Task_Fanfare(u8 taskId)
{
    if (sFanfareCounter)
    {
        sFanfareCounter--;
    }
    else
    {
        m4aMPlayContinue(&gMPlayInfo_BGM);
        DestroyTask(taskId);
    }
}

static void CreateFanfareTask(void)
{
    if (FuncIsActiveTask(Task_Fanfare) != TRUE)
        CreateTask(Task_Fanfare, 80);
}

void FadeInNewBGM(u16 songNum, u8 speed)
{
    if (gDisableMusic)
        songNum = 0;
    if (songNum == MUS_NONE)
        songNum = 0;
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_BGM);
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 0);
    m4aSongNumStop(songNum);
    m4aMPlayFadeIn(&gMPlayInfo_BGM, speed);
}

void FadeOutBGMTemporarily(u8 speed)
{
    sBGMFadeProtection = 0;
    m4aMPlayFadeOutTemporarily(&gMPlayInfo_BGM, speed);
}

bool8 IsBGMPausedOrStopped(void)
{
    if (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
        return TRUE;
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return TRUE;
    return FALSE;
}

void FadeInBGM(u8 speed)
{
    m4aMPlayFadeIn(&gMPlayInfo_BGM, speed);
}

void FadeOutBGM(u8 speed)
{
    m4aMPlayFadeOut(&gMPlayInfo_BGM, speed);
}

bool8 IsBGMStopped(void)
{
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return TRUE;
    return FALSE;
}

void PlayCry_Normal(u16 species, s8 pan)
{
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
    PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, CRY_MODE_NORMAL);
    gPokemonCryBGMDuckingCounter = 2;
    RestoreBGMVolumeAfterPokemonCry();
}

void PlayCry_NormalNoDucking(u16 species, s8 pan, s8 volume, u8 priority)
{
    PlayCryInternal(species, pan, volume, priority, CRY_MODE_NORMAL);
}

// Assuming it's not CRY_MODE_DOUBLES, this is equivalent to PlayCry_Normal except it allows other modes.
void PlayCry_ByMode(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
        gPokemonCryBGMDuckingCounter = 2;
        RestoreBGMVolumeAfterPokemonCry();
    }
}

// Used when releasing multiple Pokémon at once in battle.
void PlayCry_ReleaseDouble(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        if (!(gBattleTypeFlags & BATTLE_TYPE_MULTI))
            m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
}

// Duck the BGM but don't restore it. Not present in R/S
void PlayCry_DuckNoRestore(u16 species, s8 pan, u8 mode)
{
    if (mode == CRY_MODE_DOUBLES)
    {
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    }
    else
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
        PlayCryInternal(species, pan, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
        gPokemonCryBGMDuckingCounter = 2;
    }
}

void PlayCry_Script(u16 species, u8 mode)
{
    m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 85);
    PlayCryInternal(species, 0, CRY_VOLUME, CRY_PRIORITY_NORMAL, mode);
    gPokemonCryBGMDuckingCounter = 2;
    RestoreBGMVolumeAfterPokemonCry();
}

void PlayCryInternal(u16 species, s8 pan, s8 volume, u8 priority, u8 mode)
{
    bool32 reverse;
    u32 release;
    u32 length;
    u32 pitch;
    u32 chorus;
    u32 index;
    u8 table;

    species--;

    // Set default values
    // May be overridden depending on mode.
    length = 140;
    reverse = FALSE;
    release = 0;
    pitch = 15360;
    chorus = 0;

    switch (mode)
    {
    case CRY_MODE_NORMAL:
        break;
    case CRY_MODE_DOUBLES:
        length = 20;
        release = 225;
        break;
    case CRY_MODE_ENCOUNTER:
        release = 225;
        pitch = 15600;
        chorus = 20;
        volume = 90;
        break;
    case CRY_MODE_HIGH_PITCH:
        length = 50;
        release = 200;
        pitch = 15800;
        chorus = 20;
        volume = 90;
        break;
    case CRY_MODE_ECHO_START:
        length = 25;
        reverse = TRUE;
        release = 100;
        pitch = 15600;
        chorus = 192;
        volume = 90;
        break;
    case CRY_MODE_FAINT:
        release = 200;
        pitch = 14440;
        break;
    case CRY_MODE_ECHO_END:
        release = 220;
        pitch = 15555;
        chorus = 192;
        volume = 70;
        break;
    case CRY_MODE_ROAR_1:
        length = 10;
        release = 100;
        pitch = 14848;
        break;
    case CRY_MODE_ROAR_2:
        length = 60;
        release = 225;
        pitch = 15616;
        break;
    case CRY_MODE_GROWL_1:
        length = 15;
        reverse = TRUE;
        release = 125;
        pitch = 15200;
        break;
    case CRY_MODE_GROWL_2:
        length = 100;
        release = 225;
        pitch = 15200;
        break;
    case CRY_MODE_WEAK_DOUBLES:
        length = 20;
        release = 225;
        // fallthrough
    case CRY_MODE_WEAK:
        pitch = 15000;
        break;
    }

    SetPokemonCryVolume(volume);
    SetPokemonCryPanpot(pan);
    SetPokemonCryPitch(pitch);
    SetPokemonCryLength(length);
    SetPokemonCryProgress(0);
    SetPokemonCryRelease(release);
    SetPokemonCryChorus(chorus);
    SetPokemonCryPriority(priority);

    // This is a fancy way to get a cry of a Pokémon.
    // It creates 4 sets of 128 mini cry tables.
    // If you wish to expand Pokémon, you need to
    // append new cases to the switch.
    species = SpeciesToCryId(species);
    index = species % 128;
    table = species / 128;

    #define GET_CRY(speciesIndex, tableId, reversed) \
        ((reversed) ? &gCryTable_Reverse[(128 * (tableId)) + (speciesIndex)] : &gCryTable[(128 * (tableId)) + (speciesIndex)])

    switch (table)
    {
    case 0:
        gMPlay_PokemonCry = SetPokemonCryTone(GET_CRY(index, 0, reverse));
        break;
    case 1:
        gMPlay_PokemonCry = SetPokemonCryTone(GET_CRY(index, 1, reverse));
        break;
    case 2:
        gMPlay_PokemonCry = SetPokemonCryTone(GET_CRY(index, 2, reverse));
        break;
    case 3:
        gMPlay_PokemonCry = SetPokemonCryTone(GET_CRY(index, 3, reverse));
        break;
    }

    #undef GET_CRY
}

bool8 IsCryFinished(void)
{
    if (FuncIsActiveTask(Task_DuckBGMForPokemonCry) == TRUE)
    {
        return FALSE;
    }
    else
    {
        ClearPokemonCrySongs();
        return TRUE;
    }
}

void StopCryAndClearCrySongs(void)
{
    m4aMPlayStop(gMPlay_PokemonCry);
    ClearPokemonCrySongs();
}

void StopCry(void)
{
    m4aMPlayStop(gMPlay_PokemonCry);
}

bool8 IsCryPlayingOrClearCrySongs(void)
{
    if (IsPokemonCryPlaying(gMPlay_PokemonCry))
    {
        return TRUE;
    }
    else
    {
        ClearPokemonCrySongs();
        return FALSE;
    }
}

bool8 IsCryPlaying(void)
{
    if (IsPokemonCryPlaying(gMPlay_PokemonCry))
        return TRUE;
    else
        return FALSE;
}

static void Task_DuckBGMForPokemonCry(u8 taskId)
{
    if (gPokemonCryBGMDuckingCounter)
    {
        gPokemonCryBGMDuckingCounter--;
        return;
    }

    if (!IsPokemonCryPlaying(gMPlay_PokemonCry))
    {
        m4aMPlayVolumeControl(&gMPlayInfo_BGM, TRACKS_ALL, 256);
        DestroyTask(taskId);
    }
}

static void RestoreBGMVolumeAfterPokemonCry(void)
{
    if (FuncIsActiveTask(Task_DuckBGMForPokemonCry) != TRUE)
        CreateTask(Task_DuckBGMForPokemonCry, 80);
}

void PlayBGM(u16 songNum)
{
    if (gDisableMusic)
        songNum = 0;
    if (songNum == MUS_NONE)
        songNum = 0;
    m4aSongNumStart(songNum);
}

void PlaySE(u16 songNum)
{
    m4aSongNumStart(songNum);
}

void PlaySE12WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE1);
    m4aMPlayImmInit(&gMPlayInfo_SE2);
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

void PlaySE1WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE1);
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
}

void PlaySE2WithPanning(u16 songNum, s8 pan)
{
    m4aSongNumStart(songNum);
    m4aMPlayImmInit(&gMPlayInfo_SE2);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

void SE12PanpotControl(s8 pan)
{
    m4aMPlayPanpotControl(&gMPlayInfo_SE1, TRACKS_ALL, pan);
    m4aMPlayPanpotControl(&gMPlayInfo_SE2, TRACKS_ALL, pan);
}

bool8 IsSEPlaying(void)
{
    if ((gMPlayInfo_SE1.status & MUSICPLAYER_STATUS_PAUSE) && (gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_PAUSE))
        return FALSE;
    if (!(gMPlayInfo_SE1.status & MUSICPLAYER_STATUS_TRACK) && !(gMPlayInfo_SE2.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}

bool8 IsBGMPlaying(void)
{
    if (gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_PAUSE)
        return FALSE;
    if (!(gMPlayInfo_BGM.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}

bool8 IsSpecialSEPlaying(void)
{
    if (gMPlayInfo_SE3.status & MUSICPLAYER_STATUS_PAUSE)
        return FALSE;
    if (!(gMPlayInfo_SE3.status & MUSICPLAYER_STATUS_TRACK))
        return FALSE;
    return TRUE;
}
