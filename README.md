Invent Class Project Arduino Code

Basic Logic Flow 

setup()
    
    SET_SPRAYS_LEFT_TO_MAX

    ATTACH_DO_RESET_TO_PIN_1_FALL

    ATTACH_DO_SPRAY_COUNT_TO_PIN_2_FALL


loop()

if SPRAYS_ARE_LEFT then

    WAIT_1_MINUTE

    START_SPRAY_MOTOR 

    WAIT_30_SECONDS

    STOP_SPRAY_MOTOR

    REDUCE_SPRAYS_LEFT

    DISPLAY_REMAINING_SPRAYS 

    WAIT_1_MINUTE

    TURN_OFF_DISPLAY

    WAIT_2_WEEKS

endif


doReset()

    SET_SPRAYS_LEFT_TO_MAX


doDispaly()

    DISPLAY_REMAINING_SPRAYS

    WAIT_10_SECONDS

    TURN_OFF_DISPLAY
