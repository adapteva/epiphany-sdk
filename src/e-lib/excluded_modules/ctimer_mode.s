/* ------------------------------------------------------------------------
   Function:

   Author:
      P.Power   (july 2010)

   Modifications:

   Register usage map:

   ------------------------------------------------------------------------ */
.file    "ctimer_mode.s";


// ------------------------------------------------------------------------
.section .text;
.type    _e_ctimer_mode, %function;
.global  _e_ctimer_mode;

.balign 4;
_e_ctimer_mode:

        and   r0, r0, r0;                    // set the status to check which timer register
    //----
        beq   _ctimer0_mode;                 // jump to code for timer0


.balign 4;
_ctimer1_mode:

        movfs r1, config;                    // get the current config setting
    //----
        mov   r0, 0xf00;                     // load mask for the config register
    //----
        and   r3, r1, r0;                    // apply the mask to clear TIMERMODE bits from previous config setting
    //----
        lsr   r0, r3, 0x8;                   // shift the ctimer config bits to give 4 bit ctimer mode
    //----
        rts;                                 // return with the TIMERMODE setting

.balign 4;
_ctimer0_mode:

        movfs r1, config;                    // get the current config setting
    //----
        mov   r0, 0xf0;                      // load mask for the config register
    //----
        and   r3, r1, r0;                    // apply the mask to clear TIMERMODE bits from previous config setting
    //----
        lsr   r0, r3, 0x4;                   // shift the ctimer config bits to give 4 bit ctimer mode
    //----
        rts;                                 // return with the TIMERMODE setting

.size    _e_ctimer_mode, .-_e_ctimer_mode;


/* ------------------------------------------------------------------------
   End of File
   ------------------------------------------------------------------------ */

