#ifndef moca_assembler_parser_h
#define moca_assembler_parser_h
#include "lexer.hpp"
#include "preprocessor.hpp"

using namespace MocaAssembler_Variables;
using namespace MocaAssembler_PreProcessor;

namespace MocaAssembler_Parser
{
    class parser : private Preprocessor
    {
    private:
        usint32 variable_id = 0;
        std::vector<uslng> p_lines_to_ignore{0};
        bool has_code = false;
        Assembler *passembler;

        /* Temporary.
         * TODO: Remove. */
        constexpr p_usint8 decipher_size(usint8 size)
        {
            switch(size)
            {
                case 0: return (p_usint8)"db";break;
                case 1: return (p_usint8)"dw";break;
                case 2: return (p_usint8)"dd";break;
                default: break;
            }

            return (p_usint8)"unknown";
        }

        usint8 last_var_data_size = 0;

        /* This function is invoked upon a `mov` instruction working with a
         * imm[8,16,32] value; furthermore, it is explicitly used when a
         * imm[8,16,32] value is a variable.
         *  */
        bool parser_check_for_ignored_line(usint8 increment)
        {
            uslng line = get_line() + increment;
            
            for(auto i = p_lines_to_ignore.cbegin(); i != p_lines_to_ignore.cend(); i++)
                if(*i == line)
                {
                    switch(assembler_get_bit_type())
                    {
                        case BitType::bit16: {
                            struct variable_info<usint16> vdata = get_variable_by_address<usint16>(
                            	last_var_data_size + get_program_counter() + get_program_origin(), 
                            	(usint8)assembler_get_bit_type());
                            
                            /* Debug.
                             * TODO: Remove and complete this function.
                             * */
                            printf("%X", vdata.var_data.variable_address + (vdata.var_data.variable_size + 1));
                            exit(EXIT_SUCCESS);

                            break;
                        }
                        case BitType::bit32: {
                            break;
                        }
                        default: break;
                    }

                    variable_id++;
                    return true;
                }

            return false;
        }

        void parse_instruction(token& tok)
        {
            switch(tok.get_token_id())
            {
                case (usint8)InstructionTokens::I_mov: {
                    passembler->assembler_init_new_instruction(instructions::mov);

                    if(seek_and_test(1, '['))
                    {
                        set_token_types_to_expect(TokenTypes::variable_declaration, TokenTypes::Empty);

                        attempt_get_expected_token(tok, get_line(), 0, false);

                        /* Debug.
                         * TODO: Finish the code that is in accordance to a mov instruction working
                         * with an imm[8/16/32] value.
                         * */
                        exit(EXIT_SUCCESS);
                    }

                    set_token_types_to_expect(TokenTypes::register_tokens, TokenTypes::general_tokens);

                    /* TODO: Call to assembler to add `mov` byte. */
                    attempt_get_expected_token(tok, get_line());
                    passembler->assembler_set_lval(tok.get_token_id());

                    set_token_types_to_expect(TokenTypes::grammar_tokens, TokenTypes::Empty);
                    
                    attempt_get_expected_token(tok, get_line(), ',');
                    set_token_types_to_expect(TokenTypes::datatype_tokens, TokenTypes::register_tokens);
                    
                    /* This has to be done otherwise, for some reason, `passembler->idata` does
                     * not keep its values.
                     * */
                    struct instruction_data idata_copied = *passembler->idata;
                    delete passembler->idata;

                    attempt_get_expected_token(tok, get_line());

                    if(tok.get_token_type_id() == TokenTypes::datatype_tokens)
                    {
                        std::string value{(cp_int8)tok.get_token_value()};
                        switch(tok.get_token_id())
                        {
                            case (usint8)DataTypeTokens::DT_hex:
                            {
                                passembler->assembler_set_rval_imm(
                                    idata_copied,
                                    std::stol(value, nullptr, 0x10),
                                    get_line(),
                                    get_filename());

                                break;
                            }
                            case (usint8)DataTypeTokens::DT_dec:
                            {
                                std::cout << "Dec" << std::endl;
                                break;
                            }
                            default: moca_assembler_error(InvalidDataTypeToken, "Invalid Data Type Token.\n")
                        }

                        passembler->write_instruction_to_bin();

                        break;
                    }

                    /* Register Token. */
                    
                    break;
                }
                default: break;
            }
        }

        void parse_variable_declaration(token& tok) noexcept
        {
            usint8 var_type = tok.get_token_id();

            /* Copy over the user-define variable name to save it for adding it to the "variable stack". */
            usint8 var_name[strlen((cp_int8)tok.get_token_value())];
            strcpy((p_int8)var_name, (cp_int8)tok.get_token_value());

            /* Following the variable name should be the variable type (`db`, `dw`, `dd`). */
            tok = try_get_token<DataTypeTokens>();
            usint8 var_dt = tok.get_token_id();

            set_token_types_to_expect(TokenTypes::datatype_tokens, TokenTypes::Empty);
            attempt_get_expected_token(tok, get_line());

            /* Get the token value and store it as a string. */
            std::string var_val{(cp_int8)tok.get_token_value()};

            const auto check_size = [this, &var_dt, &var_name, &var_val](bool is_hex = false)
            {
                /* Convert the value depending on if it's a hex value or a decimal value.
                 * */
                uslng value = 0;
                if(is_hex) value = std::stol(var_val, nullptr, 0x10);
                else value = std::stol(var_val, nullptr, 0xA);

                if(var_dt == (usint8)DataTypeTokens::DT_db)
                    moca_assembler_assert(
                        value <= max_byte_size,
                        OverflowError,
                        "[Overflow Error]\n\tOn line %ld in `%s`, the variable `%s` surpasses the max size of a byte (max size: 0x%X (%d), assignment: 0x%lX (%ld)).\n",
                        get_line(), get_filename(), var_name, max_byte_size, max_byte_size, value, value
                    )
                
                if(var_dt == (usint8)DataTypeTokens::DT_dw)
                    moca_assembler_assert(
                        value <= max_word_size,
                        OverflowError,
                        "[Overflow Error]\n\tOn line %ld in `%s`, the variable `%s` surpasses the max size of a word (max size: 0x%X (%d), assignment: 0x%lX (%ld)).\n",
                        get_line(), get_filename(), var_name, max_word_size, max_word_size, value, value
                    )
                
                if(var_dt == (usint8)DataTypeTokens::DT_dd)
                    moca_assembler_assert(
                        value <= max_dword_size,
                        OverflowError,
                        "[Overflow Error]\n\tOn line %ld in `%s`, the variable `%s` surpasses the max size of a dword (max size: 0x%X (%d), assignment: 0x%lX (%ld)).\n",
                        get_line(), get_filename(), var_name, max_dword_size, max_dword_size, value, value
                    )
            };
            
            /* Decipher whether the value is a hex value or not. */
            switch(tok.get_token_id())
            {
                case (usint8)DataTypeTokens::DT_hex: {
                    if(var_type == (usint8)VariableDeclaration::VarDec)
                    {
                        check_size(true);
                        
                        add_variable<usint8>(var_name, var_dt, std::stol(var_val, nullptr, 0x10), (usint8)assembler_get_bit_type());
                        break;
                    }

                    /* Sub-variable declaration. */
                    add_subset_variable<usint8>(var_name, var_dt, std::stol(var_val, nullptr, 0xA));
                    break;
                }
                case (usint8)DataTypeTokens::DT_dec: {
                    if(var_type == (usint8)VariableDeclaration::VarDec)
                    {
                        check_size(false);

                        add_variable<usint8>(var_name, var_dt, std::stol(var_val, nullptr, 0x10), (usint8)assembler_get_bit_type());
                        break;
                    }
                    break;
                }
            }

            p_lines_to_ignore.push_back(get_line());
        }

        void parser_next(token& tok)
        {
            while(tok.get_token_id() != (usint8)GeneralTokens::GK_eof && get_line() != get_max_line())
            {
                if((tok = try_get_token<InstructionTokens>()).get_token_value() != nullptr)
                {
                    parse_instruction(tok);

                    continue;
                }

                if((tok = try_get_token<DataTypeTokens>()).get_token_value() != nullptr)
                {
                    /* See preprocessor.hpp line 187. */

                    continue;
                }

                if((tok = try_get_token<GeneralTokens>()).get_token_value() != nullptr)
                {
                    switch(tok.get_token_id())
                    {
                        case (usint8)GeneralTokens::GK_use16: break;
                        case (usint8)GeneralTokens::GK_use32: break;
                        case (usint8)GeneralTokens::GK_org: {
                            /* This still has to be done, otherwise an error will be thrown. */
                            tok = try_get_token<DataTypeTokens>();
 
                            break;
                        }
                        default: break;
                    }
                }
            }

            write_to_binary();
        }

    public:
        parser(cp_int8 filename)
            : Preprocessor(filename), passembler(nullptr)
        {
            token tok;
            while((tok = try_get_token<VariableDeclaration>()).get_token_value() != nullptr)
            {
                /* Although it seems as though this is not needed, it very much is.
                 * Reasons being: some assembly files could be full of just variable declarations, thus on the
                 * last variable declaration we could run into an EOF. 
                 * */
                if(tok.get_token_id() == (usint8)GeneralTokens::GK_eof)
                    break;

                parse_variable_declaration(tok);
            }
            
            start_preprocessor(p_lines_to_ignore, tok, has_code);

            passembler = &return_assembler();
            parser_next(tok);
        }

        ~parser() = default;
    };
}
#endif
