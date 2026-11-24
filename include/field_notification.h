#ifndef GUARD_FIELD_NOTIFICATION_H
#define GUARD_FIELD_NOTIFICATION_H

// The popups that slide in at the top of the overworld. They all move by scrolling BG0, so
// they are shown one at a time, from a queue.
//
// To add one: give it an id here, describe it in sTypes (src/field_notification.c) and call
// QueueFieldNotification wherever it happens. That can be anywhere, a battle included; it is
// shown once the player is back in the overworld and free to move.
enum
{
    NOTIF_MAP_NAME,
    NOTIF_CHAIN_REROLL,
    NOTIF_NEW_MOVES,
    NOTIF_PICKUP_ITEM,
    NOTIF_COUNT
};

struct Pokemon;

// id and arg are up to the type. Queueing a notification again while one with its type and id
// is still waiting only updates that one's arg.
void QueueFieldNotification(u8 type, u32 id, u16 arg);
// For notifications about a party Pokémon. They follow it if the party is reordered, and are
// dropped if it has left the party by the time they would be shown.
void QueueMonFieldNotification(u8 type, struct Pokemon *mon, u16 arg);
// The item the Pokémon is now holding is one it found. While its popup is on screen, the
// A Button puts the item in the Bag.
void QueuePickupItemNotification(struct Pokemon *mon);

// For an A Button press with the player free to move. TRUE if the popup on screen had a use
// for it, in which case nothing else is to act on the press.
bool8 PressAOnFieldNotification(void);

// TRUE while a notification is on screen or waiting to be
bool8 AreFieldNotificationsPending(void);
// Takes the popup off the screen at once, for anything else that is about to use BG0
void HideFieldNotifications(void);
void UpdateFieldNotifications(void);
void ResetFieldNotificationGfx(void);

#endif // GUARD_FIELD_NOTIFICATION_H
