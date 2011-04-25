/* fsa9480 usb switch driver
 *
 */

#ifndef _LINUX_FSA9480_H
#define _LINUX_FSA9480_H

#ifdef __KERNEL__

/* Status */
#define STATUS_BATTERY		0x00
#define STATUS_USB		0x04
#define STATUS_AC		0x40
#define STATUS_AC2		0x10

int get_cable_status(void);

#endif /* __KERNEL__ */

#endif /* _LINUX_FSA9480_H */

