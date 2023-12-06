#ifndef moca_assembler_preprecessor
#define moca_assembler_preprecessor
#include "assembler/moca_assembler.hpp"
#include "lexer.hpp"

using namespace MocaAssembler_Lexer;
using namespace MocaAssembler;

namespace MocaAssembler_PreProcessor
{
    class Preprocessor : protected Assembler, protected lexer
    {
    private:
        /* Used to find the according variable that is getting assigned a memory
         * address.
         * */
        usint8 variable_id = 0;

        /* Used to decipher whether or not the first two lines need to be skipped or not.
         *
         * The first line/second line will need to be skipped thereafter a `use16` and/or a `org`
         * directive residing on the first few lines.
         * */
        bool skip_first_line = false;
        bool skip_second_line = false;

        std::vector<uslng> counters{0};

        void assign_mem_address()
        {
            add_mem_address_to_variable(
                variable_id,
                counters[counters.size()-1],
                get_program_origin()
            );
            variable_id++;
        }

        void preprocessor_parse_instruction(token& tok, std::vector<uslng> p_lines_to_ignore)
        {
            redo:
            switch(tok.get_token_id())
            {
                case (usint8)InstructionTokens::I_mov: {
                    if(seek_and_test(1, '['))
                    {
                        set_token_types_to_expect(TokenTypes::variable_declaration, TokenTypes::Empty);

                        /* imm[8,16,32] */
                        attempt_get_expected_token(tok, get_line(), 0, false);

                        const auto perform_check = [this, &tok](const cp_int8 array[], uslng array_size, cp_int8 err_msg)
                        {
                            for(uslng i = 0; i < array_size; i++)
                                moca_assembler_assert(
                                    !(strcmp((cp_int8)tok.get_token_value(), array[i]) == 0),
                                    InvalidToken,
                                    err_msg,
                                    get_line(), get_filename(), array[i])
                        };

                        /* Make sure it's not a register token value. */
                        perform_check(
                            register_token_values,
                            sizeof(register_token_values)/sizeof(register_token_values[0]),
                            "[Invalid Token]\n\tOn line %ld in `%s`, there is an attempt to use a register (`%s`) as a memory operand.\n");
                        
                        /* Make sure it's not a general token value. */
                        perform_check(
                            general_token_values,
                            sizeof(general_token_values)/sizeof(general_token_values[0]),
                            "[Invalid Token]\n\tOn line %ld in `%s`, there is an attempt to use a directive (`%s`) as a memory operand.\n");
                        
                        /* Make sure it's not a data type token (`db`, `dw`, `dd`). */
                        perform_check(
                            data_type_token_values,
                            sizeof(data_type_token_values)/sizeof(data_type_token_values[0]),
                            "[Invalid Token]\n\tOn line %ld in `%s`, there is an attempt to use a data type value (`%s`) as a memory operand.\n");
                        
                        /* Debug.
                         * TODO: Finish the code that is in accordance to a mov instruction working
                         * with an imm[8/16/32] value.
                         * */
                        exit(EXIT_SUCCESS);
                    }

                    set_token_types_to_expect(TokenTypes::register_tokens, TokenTypes::general_tokens);

                    increment_program_counter(); /* `mov` */
                    attempt_get_expected_token(tok, get_line());
                    set_token_types_to_expect(TokenTypes::grammar_tokens, TokenTypes::Empty);
                    
                    increment_program_counter(); /* register */
                    attempt_get_expected_token(tok, get_line(), ',');
                    set_token_types_to_expect(TokenTypes::datatype_tokens, TokenTypes::register_tokens);

                    if(seek_and_return(1) == '[')
                    {
                        lexer_advance();
                        increment_program_counter();
                        
                        while(get_current_char() != ']')
                            lexer_advance();

                        lexer_advance();
                        goto end;
                    }

                    attempt_get_expected_token(tok, get_line());
                    increment_program_counter(); /* value */

                    end:
                    counters.push_back(get_program_counter());

                    break;
                }
                default: break;
            }
        }

        void preprocessor_checks(token& tok, std::vector<uslng>& p_lines_to_ignore, bool has_code = true)
        {
            /* If there is no (presumable) code, just return.
             * Code occurrence is deciphered in the parser.
             * */
            if(!has_code)
                return;

            while(tok.get_token_id() != (usint8)GeneralTokens::GK_eof && get_current_char() != '\0')
            {
                /* This is critical. If there is no `use16`/`use32` directive found on the
                 * first/second line, we will safely assume the assembly program is 32-bit.
                 * */
                if(assembler_get_bit_type() == BitType::NoneSet && (get_line()) > 1)
                    assembler_set_bit_type(BitType::bit32);

                if(seek_and_return(1) == '\0')
                    break;

                if(get_current_char() == '\n')
                {
                    while(get_current_char() == '\n') lexer_advance();
                    

                    if(get_current_char() == '!' || (get_current_char() == ' ' && seek_and_return(4) == '.'))
                    {
                        assign_mem_address();

                        while((seek_and_return(1) != '\n' && seek_and_return(1) != '\0') &&
                              (get_current_char() != '\n' && get_current_char() != '\0'))
                            lexer_advance();

                        lexer_advance();
                    }

                    if(get_line() == get_max_line())
                        break;

                    continue;
                }

                if((tok = try_get_token<InstructionTokens>()).get_token_value() != nullptr)
                {
                    preprocessor_parse_instruction(tok, p_lines_to_ignore);

                    continue;
                }

                if((tok = try_get_token<DataTypeTokens>()).get_token_value() != nullptr)
                {
                    /* TODO: Implement code to deal with a datatype token.
                     *
                     * Odds are, this will deal with standalone `db`/`dw`/`dd` ordeals, thus
                     * this statement will invoke assembler-specific functionality to manually insert
                     * a byte, 2-byte or 4-byte value into the overall binary being generated.
                     * */

                    continue;
                }

                if((tok = try_get_token<GeneralTokens>()).get_token_value() != nullptr)
                {
                    switch(tok.get_token_id())
                    {
                        case (usint8)GeneralTokens::GK_use16:skip_first_line = true;break;
                        case (usint8)GeneralTokens::GK_use32:break;
                        case (usint8)GeneralTokens::GK_org:
                        {
                            skip_second_line = true;
                            /* This still has to be done, otherwise an error will be thrown. */
                            tok = try_get_token<DataTypeTokens>(0, true, true);
                            break;
                        }
                        default:break;
                    }
                }
            }
        }
    
    protected:

        /* Limit the amount of plausible lexer-related errors to occur by specifying the
         * type of tokens we expect to get, and thereafter the token value being nullptr the
         * preprocessor/parser can safely assume the required token was not valid on the given line.
         * */
        TokenTypes token_type_to_expect[2];
        constexpr void set_token_types_to_expect(TokenTypes t1, TokenTypes t2)
        {
            token_type_to_expect[0] = t1;
            token_type_to_expect[1] = t2;
        }

        void attempt_get_expected_token(token& tok, uslng line, usint8 expected = 0, bool var_needs_expl = true)
        {
            for(usint8 i = 0; i < 2; i++)
            {
                if(token_type_to_expect[i] == TokenTypes::Empty) break;

                switch(token_type_to_expect[i])
                {
                    case TokenTypes::instruction_tokens: tok = try_get_token<InstructionTokens>();return;
                    case TokenTypes::register_tokens: tok = try_get_token<RegisterTokens>();return;
                    case TokenTypes::datatype_tokens: tok = try_get_token<DataTypeTokens>();return;
                    case TokenTypes::general_tokens: tok = try_get_token<GeneralTokens>();return;
                    case TokenTypes::grammar_tokens: tok = try_get_token<GrammarTokens>(expected);return;
                    case TokenTypes::variable_declaration: tok = try_get_token<VariableDeclaration>(expected, var_needs_expl);return;
                    default: break;
                }
            }

            if(tok.get_token_value() == nullptr)
                moca_assembler_error(
                    InvalidToken,
                    "[Invalid Tokens]\n\tThe expected tokens were not valid on line %ld in `%s`.\n", 
                    line, get_filename())
        }

        void start_preprocessor(std::vector<uslng>& p_lines_to_ignore, token &tok, bool has_code = true)
        {
            /* The parser begins by searching through the assembly code attempting to find any and all
             * variable declarations. Thereafter, this function is called. Thus, we must reset the
             * lexers data.
             * */
            reset_lexer_data();

            while(get_current_char() != '\0')
            {
                /* Check to see if there exists any sort of code.
                 * If so, set `has_code` to true so the preprocessor knows to search
                 * for code or not.
                 *
                 * `has_code` saves a bit of time in regards to the preprocessor and the parser.
                 * If there is no code to be found, the preprocessor jumps straight into assigning 
                 * variables there according memory address; furthermore, it also prevents the 
                 * parser from doing any further work.
                 * 
                 * */
                if(seek_and_return(1) == '\n' || get_current_char() == '\n')
                {
                    while(get_current_char() == '\n')
                          lexer_advance();

                    if(is_ascii(get_current_char()))
                        has_code = true;
                }

                lexer_advance();
            }

            reset_lexer_data();

            /* Get rid of the last token state. */
            tok.reset_token_data();

            /* Perform the according checks. */
            preprocessor_checks(tok, p_lines_to_ignore, has_code);

            see_names((usint8)assembler_get_bit_type());

            reset_lexer_data();
            reset_assembler_data();
        }

    public:
        Preprocessor(cp_int8 filename)
            : lexer(filename), Assembler()
        {
            /* First things first:
             * Check to see if there is `use16`/`use32` on the first line alongside if there is
             * is an `org` directive on the second.
             * */
            token tok = try_get_token<GeneralTokens>(0, true, true); /* override = true; we do not want to check for variable declarations. */
            
            const auto check_line = [this, &tok](uslng increment = 0, uslng line)
            {
                moca_assembler_assert(
                    (get_line() + increment) == line,
                    InvalidLineForDirective,
                    "[Invalid Line For Directive]\n\tOn line %ld in `%s` dwells a unwanted directive `%s`.\n",
                    get_line()+increment, get_filename(), tok.get_token_value())
            };

            switch(tok.get_token_id())
            {
                /* `use16` and `use32` must be on the first line, if being used.
                 * `org` must be on the second line, if being used.
                 * */
                case (usint8)GeneralTokens::GK_use16: {
                    check_line(0, 1);
                    assembler_set_bit_type(BitType::bit16);
                            
                    break;
                }
                case (usint8)GeneralTokens::GK_use32: {
                    check_line(0, 1);
                    assembler_set_bit_type(BitType::bit32);
                            
                    break;
                }
                case (usint8)GeneralTokens::GK_org: {
                    /* TODO:
                     * The org directive can be on the first or second, as the `use16`/`use32`
                     * directives won't always exist in the code.
                     * Add support for having the org on the first line.
                     * */
                    set_origin:
                    check_line(0, 2);
                    tok = try_get_token<DataTypeTokens>(0, true, true);

                    /* Get the token value and pass a conversion to `set_origin`. */
                    std::string s{(cp_int8)tok.get_token_value()};
                    set_origin(std::stoi(s, nullptr, 0x10));

                    return;
                    break;
                }
                default:break;
            }

            if((tok = try_get_token<GeneralTokens>(0, true, true)).get_token_id() == (usint8)GeneralTokens::GK_org)
                goto set_origin;
        }

        ~Preprocessor() = default;
    };
}

#endif
