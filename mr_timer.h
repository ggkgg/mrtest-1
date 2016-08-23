#ifndef _MR_TIMER_H
#define _MR_TIMER_H

/* the parameter of signal timer, for mac test */
#define MT_TDELAY_S		1			/* second */
#define MT_TDELAY_US	0			/* us 0~999999 */
#define MT_TINTVL_S		0			/* second */
#define MT_TINTVL_US	500000		/* us 0~999999 */

/* for mac test */
#define MT_MAC_SSLOT	500			/* ms, mac slot  */

void mt_tsch_init();
int  mt_mac_timer();
void mt_timer_sche(int);
void mt_mac_sslot(void*);

#endif
