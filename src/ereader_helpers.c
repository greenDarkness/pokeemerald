#include "global.h"
#include "malloc.h"
#include "decompress.h"
#include "ereader_helpers.h"
#include "link.h"
#include "main.h"
#include "union_room.h"
#include "save.h"
#include "sprite.h"
#include "task.h"
#include "util.h"
#include "constants/trainers.h"
#include "constants/moves.h"
#include "constants/items.h"

struct SendRecvMgr
{
    bool8 isParent;
    u8 state;              // EREADER_XFR_STATE_*
    u8 xferState;          // EREADER_XFER_*
    u8 checksumResult;     // EREADER_CHECKSUM_*
    u8 cancellationReason; // EREADER_CANCEL_*
    u32 *data;             // Payload source or destination
    int cursor;            // Index of the next word
    int size;              // Last word index
    int checksum;
};

static void GetKeyInput(void);
static u16 DetermineSendRecvState(u8);
static void EnableSio(void);
static void DisableTm3(void);
static void SetUpTransferManager(size_t, const void *, void *);
static void StartTm3(void);

static struct SendRecvMgr sSendRecvMgr;
static u16 sJoyNewOrRepeated;
static u16 sJoyNew;
static u16 sSendRecvStatus;
static u16 sCounter1;
static u32 sCounter2;
static u16 sSavedIme;
static u16 sSavedIe;
static u16 sSavedTm3Cnt;
static u16 sSavedSioCnt;
static u16 sSavedRCnt;

int EReader_Send(int size, const void *src)
{
    int result;
    int sendStatus;

    EReaderHelper_SaveRegsState();
    while (1)
    {
        GetKeyInput();
        if (sJoyNew & B_BUTTON)
            gShouldAdvanceLinkState = 2;

        sendStatus = EReaderHandleTransfer(1, size, src, NULL);
        sSendRecvStatus = sendStatus;
        if ((sSendRecvStatus & EREADER_XFER_MASK) == 0 && sSendRecvStatus & EREADER_CHECKSUM_OK_MASK)
        {
            result = 0;
            break;
        }
        else if (sSendRecvStatus & EREADER_CANCEL_KEY_MASK)
        {
            result = 1;
            break;
        }
        else if (sSendRecvStatus & EREADER_CANCEL_TIMEOUT_MASK)
        {
            result = 2;
            break;
        }
        else
        {
            gShouldAdvanceLinkState = 0;
            VBlankIntrWait();
        }
    }

    CpuFill32(0, &sSendRecvMgr, sizeof(sSendRecvMgr));
    EReaderHelper_RestoreRegsState();
    return result;
}

int EReader_Recv(void *dest)
{
    int result;
    int recvStatus;

    EReaderHelper_SaveRegsState();
    while (1)
    {
        GetKeyInput();
        if (sJoyNew & B_BUTTON)
            gShouldAdvanceLinkState = 2;

        recvStatus = EReaderHandleTransfer(0, 0, NULL, dest);
        sSendRecvStatus = recvStatus;
        if ((sSendRecvStatus & EREADER_XFER_MASK) == 0 && sSendRecvStatus & EREADER_CHECKSUM_OK_MASK)
        {
            result = 0;
            break;
        }
        else if (sSendRecvStatus & EREADER_CANCEL_KEY_MASK)
        {
            result = 1;
            break;
        }
        else if (sSendRecvStatus & EREADER_CANCEL_TIMEOUT_MASK)
        {
            result = 2;
            break;
        }
        else
        {
            gShouldAdvanceLinkState = 0;
            VBlankIntrWait();
        }
    }

    CpuFill32(0, &sSendRecvMgr, sizeof(sSendRecvMgr));
    EReaderHelper_RestoreRegsState();
    return result;
}

static void CloseSerial(void)
{
    REG_IME = 0;
    REG_IE &= ~(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_IME = 1;
    REG_SIOCNT = 0;
    REG_TM3CNT_H = 0;
    REG_IF = INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL;
}

static void OpenSerialMulti(void)
{
    REG_IME = 0;
    REG_IE &= ~(INTR_FLAG_TIMER3 | INTR_FLAG_SERIAL);
    REG_IME = 1;
    REG_RCNT = 0;
    REG_SIOCNT = SIO_MULTI_MODE;
    REG_SIOCNT |= SIO_INTR_ENABLE | SIO_115200_BPS;
    REG_IME = 0;
    REG_IE |= INTR_FLAG_SERIAL;
    REG_IME = 1;

    if (sSendRecvMgr.state == 0)
        CpuFill32(0, &sSendRecvMgr, sizeof(sSendRecvMgr));
}

static void OpenSerial32(void)
{
    REG_RCNT = 0;
    REG_SIOCNT = SIO_32BIT_MODE | SIO_INTR_ENABLE;
    REG_SIOCNT |= SIO_MULTI_SD;
    gShouldAdvanceLinkState = 0;
    sCounter1 = 0;
    sCounter2 = 0;
}

int EReaderHandleTransfer(u8 mode, size_t size, const void *data, void *recvBuffer)
{
    switch (sSendRecvMgr.state)
    {
    case EREADER_XFR_STATE_INIT:
        OpenSerialMulti();
        sSendRecvMgr.xferState = EREADER_XFER_EXE;
        sSendRecvMgr.state = EREADER_XFR_STATE_HANDSHAKE;
        break;
    case EREADER_XFR_STATE_HANDSHAKE:
        if (DetermineSendRecvState(mode))
            EnableSio();

        if (gShouldAdvanceLinkState == 2)
        {
            sSendRecvMgr.cancellationReason = EREADER_CANCEL_KEY;
            sSendRecvMgr.state = EREADER_XFR_STATE_DONE;
        }
        break;
    case EREADER_XFR_STATE_START:
        OpenSerial32();
        SetUpTransferManager(size, data, recvBuffer);
        sSendRecvMgr.state = EREADER_XFR_STATE_TRANSFER;
        // fall through
    case EREADER_XFR_STATE_TRANSFER:
        if (gShouldAdvanceLinkState == 2)
        {
            sSendRecvMgr.cancellationReason = EREADER_CANCEL_KEY;
            sSendRecvMgr.state = EREADER_XFR_STATE_DONE;
        }
        else
        {
            sCounter1++;
            sCounter2++;
            if (!sSendRecvMgr.isParent && sCounter2 > 60)
            {
                sSendRecvMgr.cancellationReason = EREADER_CANCEL_TIMEOUT;
                sSendRecvMgr.state = EREADER_XFR_STATE_DONE;
            }

            if (sSendRecvMgr.xferState != EREADER_XFER_CHK)
            {
                if (sSendRecvMgr.isParent && sCounter1 > 2)
                {
                    EnableSio();
                    sSendRecvMgr.xferState = EREADER_XFER_CHK;
                }
                else
                {
                    EnableSio();
                    sSendRecvMgr.xferState = EREADER_XFER_CHK;
                }
            }
        }
        break;
    case EREADER_XFR_STATE_TRANSFER_DONE:
        OpenSerialMulti();
        sSendRecvMgr.state = EREADER_XFR_STATE_CHECKSUM;
        break;
    case EREADER_XFR_STATE_CHECKSUM:
        if (sSendRecvMgr.isParent == TRUE && sCounter1 > 2)
            EnableSio();

        if (++sCounter1 > 60)
        {
            sSendRecvMgr.cancellationReason = EREADER_CANCEL_TIMEOUT;
            sSendRecvMgr.state = EREADER_XFR_STATE_DONE;
        }
        break;
    case EREADER_XFR_STATE_DONE:
        if (sSendRecvMgr.xferState)
        {
            CloseSerial();
            sSendRecvMgr.xferState = 0;
        }
        break;
    }

    return (sSendRecvMgr.xferState << EREADER_XFER_SHIFT)
         | (sSendRecvMgr.cancellationReason << EREADER_CANCEL_SHIFT)
         | (sSendRecvMgr.checksumResult << EREADER_CHECKSUM_SHIFT);
}

static u16 DetermineSendRecvState(u8 mode)
{
    bool16 resp;
    if ((*(vu32 *)REG_ADDR_SIOCNT & (SIO_MULTI_SI | SIO_MULTI_SD)) == SIO_MULTI_SD && mode)
        resp = sSendRecvMgr.isParent = TRUE;
    else
        resp = sSendRecvMgr.isParent = FALSE;
    return resp;
}

static void SetUpTransferManager(size_t size, const void *data, void *recvBuffer)
{
    if (sSendRecvMgr.isParent)
    {
        REG_SIOCNT |= SIO_38400_BPS;
        sSendRecvMgr.data = (void *)data;
        REG_SIODATA32 = size;
        sSendRecvMgr.size = size / 4 + 1;
        StartTm3();
    }
    else
    {
        REG_SIOCNT = REG_SIOCNT;
        sSendRecvMgr.data = recvBuffer;
    }
}

static void StartTm3(void)
{
    REG_TM3CNT_L = -601;
    REG_TM3CNT_H = TIMER_INTR_ENABLE;
    REG_IME = 0;
    REG_IE |= INTR_FLAG_TIMER3;
    REG_IME = 1;
}

void EReaderHelper_Timer3Callback(void)
{
    DisableTm3();
    EnableSio();
}

void EReaderHelper_SerialCallback(void)
{
    u16 i, cnt1, cnt2;
    u32 recv32;
    u16 recv[4];

    switch (sSendRecvMgr.state)
    {
    case EREADER_XFR_STATE_HANDSHAKE:
        REG_SIOMLT_SEND = EREADER_HANDSHAKE;
        *(u64 *)recv = REG_SIOMLT_RECV;
        for (i = 0, cnt1 = 0, cnt2 = 0; i < 4; i++)
        {
            if (recv[i] == EREADER_HANDSHAKE)
                cnt1++;
            else if (recv[i] != 0xFFFF)
                cnt2++;
        }

        if (cnt1 == 2 && cnt2 == 0)
            sSendRecvMgr.state = 2;
        break;
    case EREADER_XFR_STATE_TRANSFER:
        recv32 = REG_SIODATA32;
        // The first value sent by the EReader is the payload size
        if (!sSendRecvMgr.cursor && !sSendRecvMgr.isParent)
            sSendRecvMgr.size = recv32 / 4 + 1;

        if (sSendRecvMgr.isParent == TRUE)
        {
            // Send mode
            if (sSendRecvMgr.cursor < sSendRecvMgr.size)
            {
                REG_SIODATA32 = sSendRecvMgr.data[sSendRecvMgr.cursor];
                sSendRecvMgr.checksum += sSendRecvMgr.data[sSendRecvMgr.cursor];
            }
            else
            {
                REG_SIODATA32 = sSendRecvMgr.checksum;
            }
        }
        else
        {
            // Receive mode
            if (sSendRecvMgr.cursor > 0 && sSendRecvMgr.cursor < sSendRecvMgr.size + 1)
            {
                sSendRecvMgr.data[sSendRecvMgr.cursor - 1] = recv32;
                sSendRecvMgr.checksum += recv32;
            }
            else if (sSendRecvMgr.cursor)
            {
                if (sSendRecvMgr.checksum == recv32)
                    sSendRecvMgr.checksumResult = EREADER_CHECKSUM_OK;
                else
                    sSendRecvMgr.checksumResult = EREADER_CHECKSUM_ERR;
            }

            sCounter2 = 0;
        }

        if (++sSendRecvMgr.cursor < sSendRecvMgr.size + 2)
        {
            if (sSendRecvMgr.isParent)
                REG_TM3CNT_H |= TIMER_ENABLE;
            else
                EnableSio();
        }
        else
        {
            sSendRecvMgr.state = EREADER_XFR_STATE_TRANSFER_DONE;
            sCounter1 = 0;
        }
        break;
    case EREADER_XFR_STATE_CHECKSUM:
        if (!sSendRecvMgr.isParent)
            REG_SIOMLT_SEND = sSendRecvMgr.checksumResult;

        *(vu64 *)recv = REG_SIOMLT_RECV;
        if (recv[1] == EREADER_CHECKSUM_OK || recv[1] == EREADER_CHECKSUM_ERR)
        {
            if (sSendRecvMgr.isParent == TRUE)
                sSendRecvMgr.checksumResult = recv[1]; // EReader has (in)validated the payload

            sSendRecvMgr.state = EREADER_XFR_STATE_DONE;
        }
        break;
    }
}

static void EnableSio(void)
{
    REG_SIOCNT |= SIO_ENABLE;
}

static void DisableTm3(void)
{
    REG_TM3CNT_H &= ~TIMER_ENABLE;
    REG_TM3CNT_L = 0xFDA7;
}

static void GetKeyInput(void)
{
    int rawKeys = REG_KEYINPUT ^ KEYS_MASK;
    sJoyNew = rawKeys & ~sJoyNewOrRepeated;
    sJoyNewOrRepeated = rawKeys;
}

void EReaderHelper_SaveRegsState(void)
{
    sSavedIme = REG_IME;
    sSavedIe = REG_IE;
    sSavedTm3Cnt = REG_TM3CNT_H;
    sSavedSioCnt = REG_SIOCNT;
    sSavedRCnt = REG_RCNT;
}

void EReaderHelper_RestoreRegsState(void)
{
    REG_IME = sSavedIme;
    REG_IE = sSavedIe;
    REG_TM3CNT_H = sSavedTm3Cnt;
    REG_SIOCNT = sSavedSioCnt;
    REG_RCNT = sSavedRCnt;
}

void EReaderHelper_ClearSendRecvMgr(void)
{
    CpuFill32(0, &sSendRecvMgr, sizeof(sSendRecvMgr));
}
