#ifndef moca_assembler_tokens_h
#define moca_assembler_tokens_h
#include "../common.hpp"

namespace MocaAssembler_Tokens
{
    enum class InstructionTokens : usint8
    {
        I_mov   = 0x00   /* `mov` */
    };

    /* TODO: Change all of these arrays into vectors. This will cut down on
     * some compile time/runtime in the slightest.
     * */
    constexpr cp_int8 instruction_token_values[] = {
        "mov"
    };

    enum class DataTypeTokens : usint8
    {
        DT_hex  = 0x01,     /* 0xABC */
        DT_dec  = 0x02,     /* 123 */
        DT_db   = 0x03,
        DT_dw   = 0x04,
        DT_dd   = 0x05,
    };

    constexpr cp_int8 data_type_token_values[] = {
        "db", "dw", "dd"
    };

    enum class RegisterTokens : usint8
    {
        R_ax = 0x06, R_ah = 0x07, R_al = 0x08,   /* ax, ah, al */
        R_bx = 0x09, R_bh = 0x0A, R_bl = 0x0B,   /* bx, bh, bl */
        R_cx = 0x0C, R_ch = 0x0D, R_cl = 0x0E,   /* cx, ch, cl */
        R_dx = 0x0F, R_dh = 0x10, R_dl = 0x11    /* dx, dh, dl */
    };

    constexpr cp_int8 register_token_values[] = {
        "ax", "ah", "al",
        "bx", "bh", "bl",
        "cx", "ch", "cl",
        "dx", "dh", "dl"
    };

    enum class GeneralTokens : usint8
    {
        GK_byte     = 0x12,     /* byte */
        GK_word     = 0x13,     /* word */
        GK_dword    = 0x14,     /* dword */
        GK_use16    = 0x15,     /* `use16` */
        GK_use32    = 0x16,     /* `use32` */
        GK_org      = 0x17,     /* `org` */
        GK_eof      = 0x18      /* End Of File (EOF) */
    };

    constexpr cp_int8 general_token_values[] = {
        "byte", "word", "dword",
        "use16", "use32",
        "org"
    };

    enum class GrammarTokens : usint8
    {
        GR_comma        = 0x16,
        GR_left_brack   = 0x17,
        GR_right_brack  = 0x18
    };

    enum class TokenTypes
    {
        instruction_tokens,
        datatype_tokens,
        register_tokens,
        general_tokens,
        grammar_tokens,
        variable_declaration,
        Empty
    };

    enum class VariableDeclaration : usint8
    {
        VarDec,
        SubVarDec,
    };

    class token
    {
    protected:
        p_usint8 token_value;
        usint8 token_id = 0;

        /* Useful for knowing what type of token we are working with, since the
         * actual token ID is referred to via a usint8 value.
         * */
        TokenTypes token_type_id;
    
    public:
        explicit token()
            : token_value(nullptr)
        {}

        explicit token(p_usint8 tval, usint8 tid, TokenTypes tt_id)
            : token_value(nullptr)
        {
            assign_token_data(tval, tid, tt_id);
        }

        /* I am being way too explicit with these templates, but oh well.
         * (it does afterall save just a few ms of compile time which is critical for assemblers).
         * */
        template<typename T>
            requires std::is_same<T, token>::value
        bool operator!=(T& tok) noexcept
        {
            if(tok.token_id == token_id &&
               strcmp((cp_int8)tok.get_token_value(), (cp_int8)token_value) == 0 &&
               tok.token_type_id == token_type_id) return false;
            
            return true;
        }

        template<typename T>
            requires std::is_same<T, token>::value
        bool operator==(T& tok) noexcept
        {
            if(tok != *this) return false;

            return true;
        }

        template<typename T>
            requires std::is_same<T, token>::value
        T& operator=(T tok) noexcept
        {
            if(this != &tok)
            {
                token_value = tok.token_value;
                token_id = tok.token_id;
                token_type_id = tok.token_type_id;
            }

            return *this;
        }

        /* Diversity :D (literally just uses the above operator overload) */
        template<typename T>
            requires std::is_same<T, token>::value
        T& operator>>(T tok) noexcept
        {
            *this = tok;

            return *this;
        }

        /* Nice to have to automatically print out all the token information. */
        template<typename T>
            requires std::is_same<T, token>::value
        friend std::ostream& operator<<(std::ostream& ostr, T& tok) noexcept
        {
            std::cout << "Token Value: " << tok.token_value  << std::endl;
            std::cout << "Token ID: " << static_cast<usint32>(tok.token_id) << std::endl;
            std::cout << "Token Type ID: " << static_cast<usint32>(tok.token_type_id) << std::endl;

            return ostr;
        }
        
        /* For debugging. Will be removed when assembler is at a sustainable point. */
        template<typename T>
            requires std::is_same<T, TokenTypes>::value
                || std::is_same<T, usint8>::value
        bool operator<<(T tt) noexcept
        {
            if(typeid(T) == typeid(TokenTypes))
            {
                switch(static_cast<usint8>(tt))
                {
                    case (usint8)TokenTypes::Empty: std::cout << "Empty" << std::endl;break;
                    case (usint8)TokenTypes::instruction_tokens: std::cout << "Instruction Token" << std::endl;break;
                    case (usint8)TokenTypes::datatype_tokens: std::cout << "Data Type Token" << std::endl;break;
                    case (usint8)TokenTypes::register_tokens: std::cout << "Register Token" << std::endl;break;
                    case (usint8)TokenTypes::general_tokens: std::cout << "General Token" << std::endl;break;
                    case (usint8)TokenTypes::grammar_tokens: std::cout << "Grammar Token" << std::endl;break;
                    case (usint8)TokenTypes::variable_declaration: std::cout << "Variable Declaration Token" << std::endl;break;
                    default: break;
                }

                return true;
            }

            if(typeid(T) == typeid(usint8))
            {
                switch(static_cast<usint8>(tt))
                {
                    case (usint8)InstructionTokens::I_mov: std::cout << "MOV Instruction" << std::endl;break;
                    case (usint8)DataTypeTokens::DT_hex: std::cout << "Hex Value" << std::endl;break;
                    case (usint8)DataTypeTokens::DT_dec: std::cout << "Decimal Value" << std::endl;break;
                    case (usint8)DataTypeTokens::DT_db: std::cout << "DB Value" << std::endl;break;
                    case (usint8)DataTypeTokens::DT_dw: std::cout << "DW Value" << std::endl;break;
                    case (usint8)DataTypeTokens::DT_dd: std::cout << "DD Value" << std::endl;break;
                    case (usint8)RegisterTokens::R_ax:
                    case (usint8)RegisterTokens::R_ah:
                    case (usint8)RegisterTokens::R_al: std::cout << "AX/AH/AL Register" << std::endl;break;
                    case (usint8)RegisterTokens::R_bx:
                    case (usint8)RegisterTokens::R_bh:
                    case (usint8)RegisterTokens::R_bl: std::cout << "BX/BH/BL Register" << std::endl;break;
                    case (usint8)RegisterTokens::R_cx:
                    case (usint8)RegisterTokens::R_ch:
                    case (usint8)RegisterTokens::R_cl: std::cout << "CX/CH/CL Register" << std::endl;break;
                    case (usint8)RegisterTokens::R_dx:
                    case (usint8)RegisterTokens::R_dh:
                    case (usint8)RegisterTokens::R_dl: std::cout << "DX/DH/DL Regsiter" << std::endl;break;
                    case (usint8)GeneralTokens::GK_byte: std::cout << "Byte Value" << std::endl;break;
                    case (usint8)GeneralTokens::GK_word: std::cout << "Word Value" << std::endl;break;
                    case (usint8)GeneralTokens::GK_dword: std::cout << "Double Word Value" << std::endl;break;
                    case (usint8)GeneralTokens::GK_use16: std::cout << "USE16 Directive" << std::endl;break;
                    case (usint8)GeneralTokens::GK_use32: std::cout << "USE32 Directive" << std::endl;break;
                    case (usint8)GeneralTokens::GK_org: std::cout << "ORG Directive" << std::endl;break;
                    case (usint8)GeneralTokens::GK_eof: std::cout << "End Of File" << std::endl;break;
                    default: break;
                }
            }

            return true;
        }

        constexpr void assign_token_data(p_usint8 tval, usint8 tid, TokenTypes tt_id)
        {
            if(token_value) free(token_value);
            token_value = tval;

            token_id = tid;
            token_type_id = tt_id;
        }

        constexpr p_usint8 get_token_value() { return token_value; }
        constexpr usint8 get_token_id() { return token_id; }
        constexpr TokenTypes get_token_type_id() { return token_type_id; }
        constexpr void reset_token_data()
        {
            token_id = (usint8)0;
            token_type_id = TokenTypes::Empty;
        }

        ~token()
        {
            if(token_value) delete token_value;
        }
    };
}

#endif