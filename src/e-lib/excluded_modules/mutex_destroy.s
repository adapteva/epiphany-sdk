/* ------------------------------------------------------------------------
   Function:

   Author:
      P.Power   (October 2010)

   Modifications:

   Register usage map:

   ------------------------------------------------------------------------ */
.file    "mutex_destroy.s"

.section .text;
.type     _e_mutex_destroy, %function
.global   _e_mutex_destroy;

.balign 4;
_e_mutex_destroy:
// ------------------------------------------------------------------------

        eor   r0, r0, r0;                    // set return value to zero
    //----
        rts;                                 // return to callee, with zero if mutex is gained or non-zero otherwise

.size	_e_mutex_destroy, .-_e_mutex_destroy

/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

