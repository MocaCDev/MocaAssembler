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

        /* Important vectors for deciphering whether or not to retain the next program counter
         * or keep with the current program counter.
         *
         * `empty_lines` are lines with absolutely no code/variable declaration.
         * `non_empty_lines` are lines with instructions (code).
         * `counters` is a vector consisting of the counter thereafter parsing through an instruction.
         *      This vector is critical for it helps decipher the memory address of each variable.
         * */
        std::vector<uslng> empty_lines{0};
        std::vector<uslng> non_empty_lines{0};
        std::vector<uslng> counters{0};

        void preprocessor_check_for_ignored_line(std::vector<uslng>& p_lines_to_ignore, uslng increment=0)
        {
            /* Make sure the first index of `counters` is zero.
             * This is critical for if the first index is not zero, the variable
             * addresses will be off.
             *
             * Normally, the first index always consists of a garbage value.
             * */
            if(counters[0] != 0) counters[0] = 0;

            uslng line = get_line();
            static uslng counters_index = 0;

            recheck_line:
            while(*std::find(
                empty_lines.cbegin(),
                empty_lines.cend(),
                line
            ) == line)
                line++;

            while(*std::find(
                non_empty_lines.cbegin(),
                non_empty_lines.cend(),
                line
            ) == line)
            {
                line++;

                if(counters_index < counters.size()-1)
                    counters_index++;
            }

            auto line_found = std::find(
                p_lines_to_ignore.cbegin(),
                p_lines_to_ignore.cend(),
                line
            );

            if(*line_found == line)
            {
                add_mem_address_to_variable(
                    variable_id,
                    counters[counters_index],
                    get_program_origin()
                );

                line++;
                variable_id++;

                goto recheck_line;
            }
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

                    attempt_get_expected_token(tok, get_line());
                    increment_program_counter(); /* value */

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

            while(tok.get_token_id() != (usint8)GeneralTokens::GK_eof)
            {
                /* This is critical. If there is no `use16`/`use32` directive found on the
                 * first/second line, we will safely assume the assembly program is 32-bit.
                 * */
                if(assembler_get_bit_type() == BitType::NoneSet && (get_line()) > 1)
                    assembler_set_bit_type(BitType::bit32);

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

            /* Perform the according checks. */
            preprocessor_checks(tok, p_lines_to_ignore, has_code);
            reset_lexer_data();

            {
                /* Find all empty/non-empty lines. */
                while(get_current_char() != '\0')
                {
                    if(get_current_char() == '\n' && seek_and_return(1) == '\n')
                    {
                        lexer_advance();

                        /* Continue to add empty lines so long as they exist. */
                        while(seek_and_return(1) == '\n')
                        {
                            empty_lines.push_back(get_line());
                            lexer_advance();
                        }
                    }

                    if(get_current_char() == '\n' && (seek_and_return(1) != '!' && seek_and_return(1) != ' '))
                    {
                        lexer_advance();

                        /* Skip the first/second line if need be. */
                        if((skip_first_line && get_line()-1 == 1) ||
                           (skip_second_line && get_line()-1 == 2))
                            continue;

                        non_empty_lines.push_back(get_line());
                    }
                    lexer_advance();
                }

                reset_lexer_data();

                /* Assign the memory addresses accordingly. */
                while(get_current_char() != '\0')
                {
                    /* If we are at the beginning of a line, figure out if the said line
                     * is one with a variable declaration; if so, the variable will get a memory address
                     * assigned to it.
                     * */
                    if(get_line_index() == 0)
                    {
                        preprocessor_check_for_ignored_line(p_lines_to_ignore, 1);
                    }

                    lexer_advance();
                }

                /* Debug info.
                 * TODO: Remove.
                 * */
                see_names((usint8)assembler_get_bit_type());

                reset_assembler_data();
                reset_lexer_data();
                tok.reset_token_data();

                /* Clear out all vectors. */
                p_lines_to_ignore.clear();
                counters.clear();
                empty_lines.clear();
                non_empty_lines.clear();

                return;
            }

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
