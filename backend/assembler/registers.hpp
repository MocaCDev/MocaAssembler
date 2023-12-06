#ifndef moca_assembler_register_value_h
#define moca_assembler_register_value_h

namespace MocaAssembler_RegisterValues
{
    class RegisterValues
    {
    protected:
        /* AX, AH, AL. */
        usint16 r_ax_value;
        usint8  r_ah_value;
        usint8  r_al_value;

        /* BX, BH, BL. */
        usint16 r_bx_value;
        usint8  r_bh_value;
        usint8  r_bl_value;

        /* CX, CH, CL. */
        usint16 r_cx_value;
        usint8  r_ch_value;
        usint8  r_cl_value;

        /* DX, DH, DL. */
        usint16 r_dx_value;
        usint8  r_dh_value;
        usint8  r_dl_value;

    public:
        explicit RegisterValues()
            : r_ax_value(0), r_ah_value(0), r_al_value(0),
              r_bx_value(0), r_bh_value(0), r_bl_value(0),
              r_cx_value(0), r_ch_value(0), r_cl_value(0),
              r_dx_value(0), r_dh_value(0), r_dl_value(0)
        {

        }

        inline constexpr usint16 get_r_ax_value() { return r_ax_value; }
        inline constexpr usint8 get_r_ah_value() { return r_ah_value; }
        inline constexpr usint8 get_r_al_value() { return r_al_value; }

        /* Parent registers:
         *      ax, bx, cx, dx.
         * */
        inline void assign_parent_register_value(usint8 reg_id, usint16 reg_value)
        {
            switch(reg_id)
            {
                case (usint8)RegisterTokens::R_ax:
                {
                    r_ax_value = reg_value;

                    /* AH - Higher (leftmost) 8 bits. */
                    r_ah_value = AssemblerCommon::LE_get_leftmost_byte(r_ax_value);
                    
                    /* AL - Lower (rightmost) 8 bits. */
                    r_al_value = AssemblerCommon::LE_get_rightmost_byte(r_ax_value);

                    printf("AX: %X\n\tAH: %X\n\tAL: %X\n",
                        r_ax_value, r_ah_value, r_al_value);
                    break;
                }
                case (usint8)RegisterTokens::R_bx:
                {
                    r_bx_value = reg_value;
                    r_bh_value = AssemblerCommon::LE_get_leftmost_byte(r_bx_value);
                    r_bl_value = AssemblerCommon::LE_get_rightmost_byte(r_bx_value);

                    printf("BX: %X\n\tBH: %X\n\tBL: %X\n",
                        r_bx_value, r_bh_value, r_bl_value);
                    break;
                }
                case (usint8)RegisterTokens::R_cx:
                {
                    r_cx_value = reg_value;
                    r_ch_value = AssemblerCommon::LE_get_leftmost_byte(r_cx_value);
                    r_cl_value = AssemblerCommon::LE_get_rightmost_byte(r_cx_value);

                    printf("CX: %X\n\tCH: %X\n\tCL: %X\n",
                        r_cx_value, r_ch_value, r_cl_value);
                    break;
                }
                case (usint8)RegisterTokens::R_dx:
                {
                    r_dx_value = reg_value;
                    r_dh_value = AssemblerCommon::LE_get_leftmost_byte(r_dx_value);
                    r_dl_value = AssemblerCommon::LE_get_rightmost_byte(r_dx_value);

                    printf("DX: %X\n\tDH: %X\n\tDL: %X\n",
                        r_dx_value, r_dh_value, r_dl_value);
                    break;
                }
                default: break;
            }
        }

        ~RegisterValues() = default;
    };
}

#endif