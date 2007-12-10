char *tecdebug_c_version = "tecdebug.c: $Revision$";

/*
 * $Date$
 * $Source$
 * $Revision$
 * $Locker$
 */

/* tecdebug.c
 * Debugging routines for Teco
 *
 *                     COPYRIGHT (c) 1985-2003 BY
 *		     PAUL CANTRELL & J. M. NISHINAGA
 *                         SUDBURY, MA 01776
 *                        ALL RIGHTS RESERVED
 *
 * This software is furnished in it's current state free of  charge.
 * The   authors   reserve  all  rights  to  the  software.  Further
 * distribution of the software is not authorized. Modifications  to
 * the  software  may  be made locally, but shall not be distributed
 * without the consent of the authors. This software  or  any  other
 * copies  thereof,  may not be provided or otherwise made available
 * to anyone without express permission of the authors. Title to and
 * ownership of this software remains with the authors.
 * 
 */

#include "teco.h"
#include "tecparse.h"

    extern struct screen_line *saved_screen;
    extern char saved_screen_valid;
    extern int term_lines;

    extern struct buff_header *buffer_headers;
    extern struct buff_header *curbuf;
    extern struct buff_header *qregister_push_down_list;

    extern struct format_line *format_line_free_list;
    extern struct format_line *format_line_alloc_list;

    extern char format_lines_invalid;

void
do_preamble_checks()
{
    tecdebug_check_screen_magic();
    tecdebug_check_buffer_magic();
    tecdebug_check_line_magic();
    tecdebug_check_format_magic();
    tecdebug_check_companion_pointers();
    tecdebug_check_window_pointers();
}

void
do_return_checks()
{
    do_preamble_checks();
}

/* TECDEBUG_CHECK_SCREEN_MAGIC - Check for matching magic numbers
 * 
 * Function:
 *
 *	This routine checks that each saved screen array element has
 *	the correct magic number.
 */
void
tecdebug_check_screen_magic()
{
register int i;
register struct screen_line *sbp;
/*
 * Basic checks first - check that each saved screen structure has the proper
 * magic number in it.
 */
    if(saved_screen_valid){
	for(sbp = saved_screen,i = 0; i < term_lines && sbp != NULL;i++,sbp++){
	    if(sbp->scr_magic != MAGIC_SCREEN){
		restore_tty();
		fprintf(
		    stderr,
		    "?saved_screen[%d] bad magic#, 0x%08x should be 0x%08x\n",
		    i,
		    sbp->scr_magic,
		    (int)MAGIC_SCREEN
		);
		CAUSE_BUS_ERROR();
	    }/* End IF */
	}/* End FOR */
    }/* End IF */
}/* End Routine */



/* TECDEBUG_CHECK_BUFFER_MAGIC - Check for matching magic numbers
 * 
 * Function:
 *
 *	This routine checks that each buffer header to check for
 *	the correct magic number.
 */
void
tecdebug_check_buffer_magic()
{
register struct buff_header *bp;
register char saw_curbuf;
register int count;

    bp = buffer_headers;
    count = 0;
    saw_curbuf = 0;
    while(bp){
	if(bp == curbuf) saw_curbuf = 1;
	if(bp->buf_magic != MAGIC_BUFFER){
	    restore_tty();
	    fprintf(
		stderr,
		"?buff_headers[%d] bad magic#, 0x%08x should be 0x%08x\n",
		count,
		bp->buf_magic,
		(int)MAGIC_BUFFER
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
	count += 1;
	bp = bp->next_header;
    }/* End While */

    if(curbuf != NULL && saw_curbuf == 0){
	restore_tty();
	fprintf(
	    stderr,
	    "?curbuf 0x%x not on buff_headers list\n",
	    (unsigned int)curbuf
	);
	CAUSE_BUS_ERROR();
    }/* End IF */

    bp = qregister_push_down_list;
    count = 0;
    while(bp){
	if(bp->buf_magic != MAGIC_BUFFER){
	    restore_tty();
	    fprintf(
		stderr,
		"?qpushdown[%d] bad magic#, 0x%08x should be 0x%08x\n",
		count,
		bp->buf_magic,
		(int)MAGIC_BUFFER
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
	count += 1;
	bp = bp->next_header;
    }/* End While */
}/* End Routine */



/* TECDEBUG_CHECK_LINE_MAGIC - Check for matching magic numbers
 * 
 * Function:
 *
 *	This routine checks that each line buffer data structure for
 *	the correct magic number.
 */
void
tecdebug_check_line_magic()
{
register struct buff_header *bp;
register struct buff_line *lbp;
register int count;

    bp = buffer_headers;
    while(bp){
	lbp = bp->first_line;
	count = 0;
	while(lbp){
	    if(lbp->lin_magic != MAGIC_LINE){
		restore_tty();
		fprintf(
		    stderr,
		    "?buf[%s] line %d bad magic#, 0x%08x should be 0x%08x\n",
		    bp->name,
		    count,
		    lbp->lin_magic,
		    (int)MAGIC_LINE
		);
		CAUSE_BUS_ERROR();
	    }/* End IF */
	    count += 1;
	    lbp = lbp->next_line;
	}/* End While */
	bp = bp->next_header;
    }/* End While */

    bp = qregister_push_down_list;
    while(bp){
	lbp = bp->first_line;
	count = 0;
	while(lbp){
	    if(lbp->lin_magic != MAGIC_LINE){
		restore_tty();
		fprintf(
		    stderr,
		    "?qreg pdl line %d bad magic#, 0x%08x should be 0x%08x\n",
		    count,
		    lbp->lin_magic,
		    (int)MAGIC_LINE
		);
		CAUSE_BUS_ERROR();
	    }/* End IF */
	    count += 1;
	    lbp = lbp->next_line;
	}/* End While */
	bp = bp->next_header;
    }/* End While */
}/* End Routine */



/* TECDEBUG_CHECK_FORMAT_MAGIC - Check for matching magic numbers
 * 
 * Function:
 *
 *	This routine checks each screen format buffer data structure for
 *	the correct magic number.
 */
void
tecdebug_check_format_magic()
{
register struct format_line *fp;
register int count;

    fp = format_line_alloc_list;
    while(fp){
	if(fp->fmt_magic != MAGIC_FORMAT){
	    restore_tty();
	    fprintf(
		stderr,
		"?format line bad magic#, 0x%08x should be 0x%08x\n",
		fp->fmt_magic,
		(int)MAGIC_FORMAT
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
	fp = fp->fmt_next_alloc;
    }/* End While */

    fp = format_line_free_list;
    count = 0;
    while(fp){
	if(fp->fmt_magic != MAGIC_FORMAT_LOOKASIDE){
	    restore_tty();
	    fprintf(
		stderr,
		"?format lookaside[%d] bad magic#, 0x%08x should be 0x%08x\n",
		count,
		fp->fmt_magic,
		(int)MAGIC_FORMAT_LOOKASIDE
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
	fp = fp->fmt_next_line;
	count += 1;
    }/* End While */

}/* End Routine */



/* TECDEBUG_CHECK_COMPANION_POINTERS - Check for pointer consistency
 * 
 * Function:
 *
 *	This routine checks that each saved_screen entry points to
 *	a single format line which also points back. It's an error
 *	if a structure points to another structure which doesn't
 *	point back.
 */
void
tecdebug_check_companion_pointers()
{
register int i;
register struct screen_line *sbp;
register struct format_line *fbp;

/*
 * First check from the saved screen side that the pointers seem to be
 * consistent - i.e. that every structure points to one and only one
 * structure which points back.
 */
    if(!saved_screen_valid) return;
    for(sbp = saved_screen,i = 0; i < term_lines && sbp != NULL;i++,sbp++){
	if(sbp->companion == NULL) continue;
	if(sbp->companion->fmt_saved_line != sbp){
	    restore_tty();
	    fprintf(
		stderr,
		"?saved_screen[%d] companion 0x%08x ->fmt_saved_line 0x%08x\n",
		i,
		(unsigned int)sbp->companion,
		(unsigned int)sbp->companion->fmt_saved_line
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
    }/* End FOR */

/*
 * Now check from the allocated format structure side of things, to make sure
 * that each one of them which points to a screen structure is pointed back
 * at by that screen structure.
 */
    fbp = format_line_alloc_list;
    while(fbp){
	if(fbp->fmt_saved_line && fbp->fmt_saved_line->companion != fbp){
	    restore_tty();
	    fprintf(
		stderr,
		"?fbp 0x%08x fbp->fmt_saved 0x%08x ->companion 0x%08x\n",
		(unsigned int)fbp,
		(unsigned int)fbp->fmt_saved_line,
		(unsigned int)fbp->fmt_saved_line->companion
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
	fbp = fbp->fmt_next_alloc;
    }/* End While */

}/* End Routine */



/* TECDEBUG_CHECK_WINDOW_POINTERS - Check for window pointer consistency
 * 
 * Function:
 *
 *	This routine checks that every format line is correctly chained with respect
 *	to which window it is connected to. An error occurs if the line_buffer does
 *	not chain down to it. Also, for every "next_window" pointer, all format
 *	structures chained off of it must belong to the same window.
 */
void
tecdebug_check_window_pointers()
{
register struct format_line *fbp,*fl_win_head,*fl_temp;
register struct window *wptr;
register struct buff_line *blp;
char saw_our_format_line;

    if(format_lines_invalid) return;

/*
 * Check each format line on the allocated list
 */
    for(fbp = format_line_alloc_list; fbp != NULL; fbp = fbp->fmt_next_alloc){
	saw_our_format_line = 0;
	blp = fbp->fmt_buffer_line;
	fl_win_head = blp->format_line;
	for(; fl_win_head != NULL; fl_win_head = fl_win_head->fmt_next_window){
	    wptr = fl_win_head->fmt_window_ptr;
	    for(fl_temp = fl_win_head; fl_temp != NULL; fl_temp = fl_temp->fmt_next_line){
		if(fl_temp == fbp) saw_our_format_line = 1;
		if(fl_temp->fmt_window_ptr != wptr){
		    restore_tty();
		    fprintf(
			stderr,
	"?wrong window in fmt chain format_line 0x%08x format_line->wptr 0x%08x wptr 0x%08x\n",
			(unsigned int)fl_temp,
			(unsigned int)fl_temp->fmt_window_ptr,
			(unsigned int)wptr
		    );
		    CAUSE_BUS_ERROR();
		}/* End IF */
		if(fl_temp->fmt_buffer_line != blp){
		    restore_tty();
		    fprintf(
			stderr,
	"?wrong buffer_line in fmt chain fmt 0x%08x fmt->fmt_buffer_line 0x%08x lbp 0x%08x\n",
			(unsigned int)fl_temp,
			(unsigned int)fl_temp->fmt_buffer_line,
			(unsigned int)blp
		    );
		    CAUSE_BUS_ERROR();
		}/* End IF */
	    }/* End FOR */
	}/* End FOR */
	if(saw_our_format_line == 0){
	    restore_tty();
	    fprintf(
		stderr,
		"?never encountered our format line 0x%08x in format list\n",
		(unsigned int)fbp
	    );
	    CAUSE_BUS_ERROR();
	}/* End IF */
    }/* End FOR */

}/* End Routine */
