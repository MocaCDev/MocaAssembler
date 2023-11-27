#ifndef moca_assembler_lexer_h
#define moca_assembler_lexer_h
#include "tokens.hpp"

using namespace MocaAssembler_Tokens;

namespace MocaAssembler_Lexer
{
    class lexer
    {
    protected:
        p_usint8 assembly_data;
        p_int8 assembly_filename;
        uslng index = 0;
        usint8 line_index = 1;
        usint8 last_line_index = line_index;
        uslng_lng assembly_data_size = 0;
        uslng line = 1;
        usint8 current_char;

        constexpr void lexer_advance(bool skip_newline = true)
        {
            if(index + 1 > assembly_data_size || assembly_data[index + 1] == '\0')
                { current_char = '\0'; return; }
            
            if(current_char == '\n')
            {
                line++;
                last_line_index = line_index;
                line_index = 0;
            } else line_index++;
            
            index++;
            current_char = assembly_data[index];
        }

        constexpr usint8 seek_and_return(uslng seek_length)
        {
            uslng curr_index = index;
            
            if(curr_index + seek_length > assembly_data_size)
                return assembly_data[assembly_data_size];
            
            curr_index += seek_length;
            return assembly_data[curr_index];
        }

        void lexer_go_back_x(uslng seek_back_length)
        {
            if(index - seek_back_length >= 0)
            {
                index -= seek_back_length;
                current_char = assembly_data[index];
                return;
            }

            index = 0;
            current_char = assembly_data[0];
        }

        constexpr void reset_lexer_data() noexcept
        {
            index = 0;
            current_char = assembly_data[index];
            line = 1;
            line_index = 0;
            last_line_index = line_index;

            if(current_char == '\n')
            {
                line++;
                lexer_advance();
            }
        }

        constexpr bool seek_and_test(uslng seek_length, usint8 test_against) noexcept
        {
            if(seek_and_return(seek_length) == test_against) return true;

            return false;
        }

        constexpr uslng get_line() { return line; }
        constexpr p_int8 get_filename() { return assembly_filename; }
        constexpr usint8 get_last_line_index() { return last_line_index; }
        constexpr usint8 get_current_char() { return current_char; }
        constexpr usint8 get_line_index() { return line_index; }

    private:

        void lexer_go_back()
        {
            if(index > 0)
            {
                index--;
                current_char = assembly_data[index];
            }
        }

        constexpr void lexer_seek_and_set(uslng seek_length)
        {
            if(index + seek_length > assembly_data_size)
            {
                index = assembly_data_size;
                current_char = assembly_data[index];

                return;
            }

            index += seek_length;
            current_char = assembly_data[index];
        }

        constexpr void check_for_whitespace(usint8 specific = 0)
        {
            if(specific != 0)
            {
                if(current_char == specific)
                    while(current_char == specific) lexer_advance();

                return;
            }

            if(current_char == ' ' || current_char == '\t' || current_char == '\n')
            {
                while(current_char == ' ' || current_char == '\t' || current_char == '\n')
                    lexer_advance();
            }
        }

        p_usint8 get_keyword(bool allow_special = false)
        {
            p_usint8 keyword = static_cast<p_usint8>(calloc(1, sizeof(*keyword)));
            usint32 i = 0;

            while(is_ascii(current_char) && !(current_char == ' ') && !(current_char == '\0'))
            {
                keyword[i] = current_char;
                i++;

                realloc:
                keyword = static_cast<p_usint8>(realloc(
                    keyword,
                    (i + 1) * sizeof(*keyword)
                ));
                lexer_advance();

                if(seek_and_test(1, '\n'))
                {
                    keyword[i] = current_char;
                    i++;
                    lexer_advance();
                    break;
                }

                if(allow_special && (current_char == '_' || is_number(current_char)))
                {
                    keyword[i] = current_char;
                    i++;
                    goto realloc;
                }
            }

            keyword[i] = '\0';

            return keyword;
        }

        p_usint8 get_hexadecimal()
        {
            p_usint8 hexadecimal_value = static_cast<p_usint8>(calloc(1, sizeof(*hexadecimal_value)));
            usint32 i = 0;

            while(is_hex(current_char) || is_number(current_char))
            {
                hexadecimal_value[i] = current_char;
                i++;

                hexadecimal_value = static_cast<p_usint8>(realloc(
                    hexadecimal_value,
                    (i + 1) * sizeof(*hexadecimal_value)
                ));
                lexer_advance();
            }

            hexadecimal_value[i] = '\0';
            return hexadecimal_value;
        }

        p_usint8 get_decimal()
        {
            p_usint8 decimal_value = static_cast<p_usint8>(calloc(1, sizeof(*decimal_value)));
            usint32 i = 0;

            while(is_number(current_char) || is_hex(current_char))
            {
                decimal_value[i] = current_char;
                i++;

                decimal_value = static_cast<p_usint8>(realloc(
                    decimal_value,
                    (i + 1) * sizeof(*decimal_value)
                ));
                lexer_advance();

                if(current_char == 'h')
                {
                    decimal_value[i] = current_char;
                    decimal_value = static_cast<p_usint8>(realloc(
                        decimal_value,
                        (i + 1) * sizeof(*decimal_value)
                    ));
                    i++;

                    lexer_advance();

                    break;
                }
            }

            decimal_value[i] = '\0';

            return decimal_value;
        }

        constexpr void attempt_to_tokenize(cp_int8 keyword, const cp_int8 token_array[], uslng token_array_size, token& tok, TokenTypes token_type, usint8 increment = 0)
        {
            for(usint8 i = 0; i < token_array_size; i++)
            {
                if(strcmp(keyword, token_array[i]) == 0)
                {
                    tok.assign_token_data((p_usint8)keyword, i + increment, token_type);
                    break;
                }
            }
        }

    public:
        lexer(cp_int8 filename)
            : assembly_data(nullptr), assembly_filename(nullptr)
        {
            assembly_filename = new int8[strlen(filename) + 1]; /* +1 to have a null-ending (\0) char pointer. */
            memcpy(assembly_filename, filename, strlen(filename));

            FILE *asm_file = fopen(filename, "rb");
            moca_assembler_assert(
                asm_file, NoSuchFile,
                "[No Such File]\n\tThe file `%s` does not exist.\n",
                filename)
            
            fseek(asm_file, 0, SEEK_END);
            assembly_data_size = ftell(asm_file);
            fseek(asm_file, 0, SEEK_SET);

            moca_assembler_assert(
                assembly_data_size > 1, NoDataInFile,
                "[No Data Detected]\n\tThe file `%s` does not seem to have any code in it.\n",
                filename)
            
            assembly_data = new usint8[assembly_data_size];
            fread(assembly_data, assembly_data_size, sizeof(*assembly_data), asm_file);
            moca_assembler_assert(
                assembly_data, DataReadInError,
                "{Read In Error]\n\tThere was an error reading in the code from `%s`.\n",
                filename)
            
            fclose(asm_file);

            current_char = assembly_data[index];
        }

        bool has_been_checked = false;
        template<typename T>
            requires std::is_same<T, InstructionTokens>::value
                || std::is_same<T, DataTypeTokens>::value
                || std::is_same<T, RegisterTokens>::value
                || std::is_same<T, GeneralTokens>::value
                || std::is_same<T, GrammarTokens>::value
                || std::is_same<T, VariableDeclaration>::value
        const token try_get_token(usint8 expected = 0, bool var_needs_expl = true, bool override = false)
        {
            token tok;
            p_usint8 keyword;

            const auto tokenize_eof = [this, &keyword, &tok]()
            {
                keyword = new usint8[1];
                keyword[0] = '\0';
                tok.assign_token_data(keyword, (usint8)GeneralTokens::GK_eof, TokenTypes::general_tokens);
            };

            if(current_char == '\0')
            {
                tokenize_eof();
                return tok;
            }

            /* Nothing should start with a number. */
            moca_assembler_assert(
                !is_number(current_char),
                InvalidDigit,
                "[Invalid Digit]\n\tLine %ld in `%s` has an invalid digit `%c`.\n", 
                get_line(), get_filename(), current_char)

            check_for_whitespace();

            const auto attempt_go_back = [this, &tok](p_usint8& kw)
            {
                if(tok.get_token_value() == nullptr)
                {
                    lexer_go_back_x(strlen((cp_int8)kw));
                    free(kw);
                    kw = nullptr;
                }
            };

            const auto perform_check = [this, &keyword, &attempt_go_back]()
            {
                if(!is_ascii(current_char)) return;

                keyword = get_keyword(true);
                bool has_been_found = false;

                for(uslng i = 0; i < sizeof(instruction_token_values)/sizeof(instruction_token_values[0]); i++)
                {
                    if(strcmp((cp_int8)keyword, instruction_token_values[i]) == 0)
                    {
                        has_been_found = true;
                        break;
                    }
                }
                if(has_been_found) { attempt_go_back(keyword); return; }

                for(uslng i = 0; i < sizeof(data_type_token_values)/sizeof(data_type_token_values[0]); i++)
                {
                    if(strcmp((cp_int8)keyword, data_type_token_values[i]) == 0)
                    {
                        has_been_found = true;
                        break;
                    }
                }
                if(has_been_found) { attempt_go_back(keyword); return; }

                for(uslng i = 0; i < sizeof(register_token_values)/sizeof(register_token_values[0]); i++)
                {
                    if(strcmp((cp_int8)keyword, register_token_values[i]) == 0)
                    {
                        has_been_found = true;
                        break;
                    }
                }
                if(has_been_found) { attempt_go_back(keyword); return; }

                for(uslng i = 0; i < sizeof(general_token_values)/sizeof(general_token_values[0]); i++)
                {
                    if(strcmp((cp_int8)keyword, general_token_values[i]) == 0)
                    {
                        has_been_found = true;
                        break;
                    }
                }
                if(has_been_found) { attempt_go_back(keyword); return; }

                moca_assembler_error(
                    UnknownError,
                    "[Unknown Error]\n\tLine %ld in `%s` dwells an unknown value `%s`.\n",
                    get_line(), get_filename(), keyword
                )
            };

            /* Check file for any unwanted values. Once this is done, don't do it again. */
            if(!has_been_checked)
            {
                while(index < assembly_data_size-1)
                {
                    if(line_index == 0) perform_check();
                    lexer_advance();
                }

                reset_lexer_data();
                has_been_checked = true;
            }

            /* So long as we are no longer "parsing" for variable declarations,
             * skip any plausible variable declaration that may be dwelling on the
             * current line.
             * */
            if(typeid(T) != typeid(VariableDeclaration) && !override)
            {
                svd1:
                if(current_char == '!' || current_char == '.')
                {
                    svd: /* Skip Variable Declaration */
                    while(current_char != '\n' && !(current_char == '\0')) lexer_advance(false);
                    lexer_advance();
                    check_for_whitespace('\t');
                    check_for_whitespace(' ');

                    /* If `\0` is the current value, tokenize the EOF. */
                    if(current_char == '\0')
                        goto teof;

                    /* Check for a current newline. */
                    if(current_char == '\n') { check_for_whitespace(); goto svd1; }

                    /* If `!` is the current value, go back to the while loop above. */
                    if(current_char == '!' || current_char == '.') goto svd;
                }
            }

            if(current_char == ';')
            {
                skip_comment:
                while(current_char != '\n') lexer_advance();
                lexer_advance();

                if(current_char == ';') goto skip_comment;
            }

            if(typeid(T) == typeid(VariableDeclaration))
            {
                if(!var_needs_expl)
                {
                    lexer_advance();
                    keyword = get_keyword();

                    tok.assign_token_data(keyword, (usint8)VariableDeclaration::VarDec, TokenTypes::variable_declaration);
                    return tok;
                }

                if(current_char != '!' && current_char != '.')
                {
                    redo:
                    while(current_char != '\n' && !(current_char == '\0') && !(current_char == '!')) lexer_advance();
                    
                    if(current_char == '\0')
                    {
                        reset_lexer_data();

                        return tok;
                    }

                    lexer_advance();

                    if(current_char == '!' || current_char == '.') goto get;

                    goto redo;
                }

                get:
                /* Decipher whether the variable is a subset variable (variable defined under another) or not. */
                usint8 t_id;
                if(current_char == '.') t_id = (usint8)VariableDeclaration::SubVarDec;
                else t_id = (usint8)VariableDeclaration::VarDec;

                /* If we get here, assuming the current character is `!`, we will advance the lexer and get the user-defined variable. */
                lexer_advance();
                keyword = get_keyword(true);

                /* Make sure there is not a newline following the user-defined variable name. */
                moca_assembler_assert(
                    current_char != '\n' && current_char != '\0',
                    InvalidNewline,
                    "[Variable Declaration Error]\n\tThere was an invalid newline/EOF token on line %ld in `%s`.\n",
                    get_line() - 1, get_filename())
                
                tok.assign_token_data(keyword, t_id, TokenTypes::variable_declaration);
                return tok;
            }

            if(typeid(T) == typeid(InstructionTokens))
            {
                /* Attempt to get an instruction token. */
                keyword = get_keyword();

                attempt_to_tokenize(
                    reinterpret_cast<cp_int8>(keyword), instruction_token_values, 
                    sizeof(instruction_token_values)/sizeof(instruction_token_values[0]), 
                    tok, TokenTypes::instruction_tokens);

                attempt_go_back(keyword);

                return tok;
            }

            if(typeid(T) == typeid(GrammarTokens))
            {
                lexer_get_grammar(tok, expected);
                return tok;
            }
            
            if(typeid(T) == typeid(DataTypeTokens))
            {
                /* Attempt to get a data type token (`db`, `dw`, `dd`). */
                keyword = get_keyword();

                attempt_to_tokenize(
                    reinterpret_cast<cp_int8>(keyword), data_type_token_values, 
                    sizeof(data_type_token_values)/sizeof(data_type_token_values[0]), 
                    tok, TokenTypes::datatype_tokens, 0x3);

                attempt_go_back(keyword);

                if(keyword == nullptr && (is_number(current_char) || is_hex(current_char)))
                {
                    if(seek_and_test(1, 'x'))
                    {
                        /* Get hexadecimal number. */
                        lexer_seek_and_set(2);
                        
                        keyword = get_hexadecimal();

                        tokenize_hex:
                        tok.assign_token_data(keyword, (usint8)DataTypeTokens::DT_hex, TokenTypes::datatype_tokens);

                        goto end_dt;
                    }

                    keyword = get_decimal();

                    if(is_hex(keyword[0]))
                    {
                        moca_assembler_assert(
                            keyword[strlen((cp_int8)keyword)-1] == 'h',
                            InvalidDigit,
                            "[Invalid Hexadecimal Value]\n\tOn line %ld in `%s` dwells a presumable hexadecimal value without an indictor.\n",
                            get_line(), get_filename())
                            
                        goto tokenize_hex;
                    }
                    
                    tok.assign_token_data(keyword, (usint8)DataTypeTokens::DT_dec, TokenTypes::datatype_tokens);

                    /* Get decimal number. */
                    return tok;
                }

                if(current_char == '\'')
                {
                    printf("Character");
                    exit(0);
                }
                
                end_dt:
                return tok;
            }

            if(typeid(T) == typeid(GeneralTokens))
            {
                /* Attempt to get a general token. */
                if(current_char == '\0')
                {
                    teof:
                    tokenize_eof();
                    goto egt;
                }

                keyword = get_keyword(true);
                
                attempt_to_tokenize(
                    reinterpret_cast<cp_int8>(keyword), general_token_values,
                    sizeof(general_token_values)/sizeof(general_token_values[0]),
                    tok, TokenTypes::general_tokens, 0x12
                );

                attempt_go_back(keyword);

                egt: /* End General Token */
                return tok;
            }

            if(typeid(T) == typeid(RegisterTokens))
            {
                if(current_char == '\0')
                    goto teof;
                
                /* Attempt to get a register token. */
                keyword = get_keyword();

                attempt_to_tokenize(
                    reinterpret_cast<cp_int8>(keyword), register_token_values, 
                    sizeof(register_token_values)/sizeof(register_token_values[0]), 
                    tok, TokenTypes::register_tokens, 6);

                attempt_go_back(keyword);

                return tok;
            }
            
            end2:
            return token();
        }

        constexpr void lexer_get_grammar(token& tok, usint8 expected)
        {
            moca_assembler_assert(
                !is_ascii(current_char),
                InvalidToken,
                "Grammar was expected on line %ld in `%s`. Instead got `%s`.\n", get_line(), get_filename(), get_keyword())
            
            moca_assembler_assert(
                current_char == expected,
                InvalidToken,
                "Grammar on line %ld in `%s` was expected to be `%c`, got `%c` instead.\n",
                get_line(), get_filename(), expected, current_char)
            
            p_usint8 token_val = new usint8[2];
            token_val[0] = current_char;
            token_val[1] = '\0';

            lexer_advance();

            tok.assign_token_data(token_val, (usint8)GrammarTokens::GR_comma, TokenTypes::grammar_tokens);
        }

        ~lexer()
        {
            if(assembly_data) delete assembly_data;
            if(assembly_filename) delete assembly_filename;
        }
    };
}

#endif