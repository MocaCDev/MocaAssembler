#ifndef moca_assembler_variables_h
#define moca_assembler_variables_h
#include "../../common.hpp"

namespace MocaAssembler_Variables
{
    /* What is the variables data type so we know the amount of memory each variable stores. */
    enum class VariableSize : usint8
    {
        VS_byte     = 0x03,
        VS_word     = 0x04,
        VS_dword    = 0x05
    };

    template<typename T>
    struct subset_variable_metadata
    {
        usint8          subset_variable_name[30];
        usint8          subset_variable_size;
        T               subset_variable_address;
    };

    template<typename T>
        requires std::is_same<T, usint16>::value
            || std::is_same<T, usint32>::value
    struct variable_metadata
    {
        usint8          variable_name[30];
        T               variable_address = 0;   /* in accordance to the program counter (set manually via `org`). */
        usint8          variable_size;

        /* A subset variable:
         *  main_variable db 0x0
         *      .subset_variable db 0x1
         * */
        std::vector<struct subset_variable_metadata<T>> subset_variables;
        usint32 subset_variables_count = 0;
        //struct subset_variable_metadata **subset_variables = nullptr;
    };

    template<typename T>
        requires std::is_same<T, usint16>::value
            || std::is_same<T, usint32>::value
    struct variable_info
    {
        bool                            is_subset_variable;
        struct variable_metadata<T>     var_data;

        union
        {
            struct subset_variable_metadata<T> sub_var_data;
        } subset_variable;

        variable_info()
            : is_subset_variable(false)
        {}

        ~variable_info() = default;
    };

    class Variables
    {
    protected:
        /* `bit16_metadata` is used for assigning data that will be stored in `bit16_variable_metadata`. */
        //struct variable_metadata<usint16> bit16_metadata;
        std::vector<struct variable_metadata<usint16>> bit16_variable_metadata;
        uslng bit16_length;

        /* `bit32_metadata` is used for assigning data that will be stored in `bit32_variable_metadata`. */
        struct variable_metadata<usint32> bit32_metadata;
        std::vector<struct variable_metadata<usint32>> bit32_variable_metadat;
        uslng bit32_length;

        template<typename T>
            requires std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        VariableSize get_variable_size(T var_address, usint8 bit_type) noexcept
        {
            switch(bit_type)
            {
                case 0x0: {
                    /* Check `bit16_variable_metadata`. */
                    for(usint32 i = 0; i < bit16_length; i++)
                    {
                        if(bit16_variable_metadata[i].variable_address == var_address) return (VariableSize)bit16_variable_metadata[i].variable_size;
                        else
                            if(bit16_variable_metadata[i].subset_variables_count > 0)
                                for(usint32 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                                    if(bit16_variable_metadata[i].subset_variables[x].subset_variable_address == var_address) return (VariableSize)bit16_variable_metadata[i].subset_variables[x].subset_variable_size;
                    }
                    break;
                }
                case 0x1: {
                    /* Check `bit32_variable_metadata`. */
                    break;
                }
                default: break;
            }

            return VariableSize::VS_byte;
        }

        template<typename T>
            requires std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        struct variable_info<T> get_variable_by_name(cp_int8 variable_name, usint8 bit_type)
        {
            struct variable_info<T> var_info;

            switch(bit_type)
            {
                case 0x0: {
                    /*for(usint32 i = 0; i < bit16_length; i++)
                    {
                        if(strcmp((cp_int8)bit16_variable_metadata[i].variable_name, variable_name) == 0) return bit16_variable_metadata[i];
                        if(bit16_variable_metadata[i].subset_variables_count > 0)
                            for(usint32 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                                if(strcmp((cp_int8)bit16_variable_metadata[i].subset_variables[x].subset_variable_name, variable_name) == 0) return bit16_variable_metadata[i];
                    }*/
                    break;
                }
                case 0x1: {
                    break;
                }
                default: break;
            }

            moca_assembler_error(
                UnknownError,
                "[Unknown Error]\n\tAn unknown error ocurred.\n")
        }

        template<typename T>
            requires std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        struct variable_info<T> get_variable_by_address(T address, usint8 bit_type)
        {
            struct variable_info<T> var_info;

            switch(bit_type)
            {
                case 0x0: {
                    for(usint32 i = 0; i < bit16_length; i++)
                    {
                        if(bit16_variable_metadata[i].variable_address == address) { var_info.var_data = bit16_variable_metadata[i]; break; }
                        if(bit16_variable_metadata[i].subset_variables_count > 0)
                            for(usint32 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                                if(bit16_variable_metadata[i].subset_variables[x].subset_variable_address == address)
                                {
                                    var_info.is_subset_variable = true;
                                    var_info.subset_variable.sub_var_data = bit16_variable_metadata[i].subset_variables[x];
                                    break;
                                }
                    }
                    /*for(usint32 i = 0; i < bit16_length; i++)
                    {
                        printf("%X", bit16_variable_metadata[i].variable_address);
                        if(bit16_variable_metadata[i].variable_address == address) return bit16_variable_metadata[i];
                        if(bit16_variable_metadata[i].subset_variables_count > 0)
                            for(usint32 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                                if(bit16_variable_metadata[i].subset_variables[x].subset_variable_address == address) return bit16_variable_metadata[i];
                    }*/
                    return var_info;
                }
                case 0x1: {
                    break;
                }
                default: break;
            }

            moca_assembler_error(
                UnknownError,
                "[Unknown Error]\n\tAn unknown error ocurred.\n")
        }

        constexpr void see_names(usint8 bit_type)
        {
            switch(bit_type)
            {
                case 0x0: {
                    /* bit16 */
                    for(usint32 i = 0; i < bit16_length; i++)
                    {
                        printf("Variable: %s, Address: %X\n",
                            bit16_variable_metadata[i].variable_name,
                            bit16_variable_metadata[i].variable_address);
                        
                        if(bit16_variable_metadata[i].subset_variables_count > 0)
                            for(usint32 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                                printf("\tSubset Variable: %s, Address: %X\n",
                                    bit16_variable_metadata[i].subset_variables[x].subset_variable_name,
                                    bit16_variable_metadata[i].subset_variables[x].subset_variable_address);
                    }
                    break;
                }
                case 0x1: {
                    /* bit32 */
                    break;
                }
                default: break;
            }
        }

        template<typename T>
            requires std::is_same<T, usint8>::value
                || std::is_same<T, p_usint8>::value
                || std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        void add_variable(p_usint8 variable_name, usint8 variable_datatype, T variable_value, usint8 bit_type = 0x2) /* 0x2 = BitType::NoneSet */
        {
            struct variable_metadata<usint16> bit16_metadata;

            const auto add_to_bit16_vector = [this, &bit16_metadata, &variable_name, &variable_datatype]()
            {
                strcpy((p_int8)bit16_metadata.variable_name, (cp_int8)variable_name);
                bit16_metadata.variable_address = bit16_length + bit32_length;
                bit16_metadata.variable_size = (usint8)variable_datatype;

                bit16_variable_metadata.push_back(bit16_metadata);
                    
                if(bit16_length > 0)
                {
                    for(usint8 i = 0; i < bit16_length; i++)
                        bit16_variable_metadata[bit16_length].variable_address += bit16_variable_metadata[i].subset_variables_count;
                }
                else
                    bit16_variable_metadata[bit16_length].variable_address += bit16_variable_metadata[bit16_length].subset_variables_count;
                    
                bit16_length++;
            };

            /* The vector of which is used, that being `bit16_variable_metadata` or `bit32_variable_metadata`
             * is dependable on the type of assembly code being assembled (16-bit or 32-bit assembly code).
             *
             * If it's 16-bit assembly code (`use16`), then the memory addresses are 2-bytes wide, else (`use32`) memory addresses are 4-bytes wide.
             *
             * TODO: Add support for checking the type of assembly code that is being assembled.
             * */
            switch(variable_datatype)
            {
                case (usint8)VariableSize::VS_byte: {
                    if(bit_type == 0x0)         /* 0x0 = BitType::bit16 */
                        add_to_bit16_vector();
                    else
                        { /* add_to_bit32_vector() */ }
                    
                    break;
                }
                case (usint8)VariableSize::VS_word: {
                    if(bit_type == 0x0)         /* 0x0 = BitType::bit16 */
                        add_to_bit16_vector();
                    else
                        { /* add_to_bit32_vector() */ }

                    break;
                }
                case (usint8)VariableSize::VS_dword: {
                    if(bit_type == 0x0)         /* 0x0 = BitType::bit16 */
                        add_to_bit16_vector();
                    else
                        { /* add_to_bit32_vector() */ }

                    break;
                }
                default: break;
            }
        }

        template<typename T>
            requires std::is_same<T, usint8>::value
                || std::is_same<T, p_usint8>::value
                || std::is_same<T, usint16>::value
                || std::is_same<T, usint32>::value
        void add_subset_variable(p_usint8 variable_name, usint8 variable_datatype, T variable_value)
        {
            struct subset_variable_metadata<usint16> svar_metadata;
            
            /* Copy over the variable name and explicitly add a null value to the end. */
            memcpy(svar_metadata.subset_variable_name, variable_name, strlen((cp_int8)variable_name));
            svar_metadata.subset_variable_name[strlen((cp_int8)variable_name)] = '\0';

            svar_metadata.subset_variable_address = bit16_length + bit32_length;
            svar_metadata.subset_variable_size = (usint8)variable_datatype;
            
            for(usint32 i = 0; i < bit16_length; i++)
                svar_metadata.subset_variable_address += bit16_variable_metadata[i].subset_variables_count;
            
            bit16_variable_metadata[bit16_length-1].subset_variables_count++;
            
            bit16_variable_metadata[bit16_length-1].subset_variables.push_back(svar_metadata);
        }

        usint32 last_program_counter = 0;
        uslng last_address = 0;
        usint8 index = 0;
        usint8 last_subset_var_id = 0;
        bool last_was_subset_var = false;

        void add_mem_address_to_variable(usint8 var_id, uslng program_counter, uslng origin)
        {
            /* This conversion is needed due to memory addresses being represented via
             * hexadecimal.
             *
             * Prior to this, lets say a variable was 14 bytes into the code. The address
             * would then be `0x14` instead of `0xE`.
             * */
            std::string laddr = std::to_string(last_address);
            last_address = std::stol(laddr, nullptr, 0xA);
            
            /* So long as there is at least one variable, decipher the size of the said variable
             * or the said subset-variable.
             * */
            if(index > 0)
            {
                if(!last_was_subset_var)
                    switch(bit16_variable_metadata[index-1].variable_size)
                    {
                        case (usint8)DataTypeTokens::DT_db: last_address += 1;break;
                        case (usint8)DataTypeTokens::DT_dw: last_address += 2;break;
                        case (usint8)DataTypeTokens::DT_dd: last_address += 4;break;
                        default: break;
                    }
                else
                    switch(bit16_variable_metadata[index-1].subset_variables[last_subset_var_id].subset_variable_size)
                    {
                        case (usint8)DataTypeTokens::DT_db: last_address += 1;last_was_subset_var = false;break;
                        case (usint8)DataTypeTokens::DT_dw: last_address += 2;last_was_subset_var = false;break;
                        case (usint8)DataTypeTokens::DT_dd: last_address += 4;last_was_subset_var = false;break;
                        default: break;
                    }
            }

            if(bit16_variable_metadata[index].variable_address == var_id)
            {
                if(last_program_counter == program_counter)
                {
                    if(last_address != 0)
                        bit16_variable_metadata[index].variable_address = (last_address + (program_counter - last_program_counter));// + bit16_variable_metadata[index-1].variable_size + 1;
                    else
                    {
                        last_address = program_counter + origin + var_id;
                        bit16_variable_metadata[index].variable_address = (last_address + (program_counter - last_program_counter));
                    }
                
                    
                    last_address = bit16_variable_metadata[index].variable_address;
                }
                else
                {
                    if(index > 0)
                        bit16_variable_metadata[index].variable_address = (last_address + ((program_counter-1) - last_program_counter));
                    else
                        bit16_variable_metadata[index].variable_address = (program_counter-1) + origin;
                }
                
                last_program_counter = program_counter;
                last_address = bit16_variable_metadata[index].variable_address;

                index++;
                return;
            }
            
            if(index > 0)
            {
                bool is_done = false;
                for(usint8 i = 0; i < index; i++)
                {
                    if(bit16_variable_metadata[i].subset_variables_count > 0)
                    {
                        for(usint8 x = 0; x < bit16_variable_metadata[i].subset_variables_count; x++)
                        {
                            if(bit16_variable_metadata[i].subset_variables[x].subset_variable_address == var_id)
                            {
                                last_was_subset_var = true;
                                if(last_program_counter == program_counter)
                                    if(x < 1)
                                        bit16_variable_metadata[i].subset_variables[x].subset_variable_address = last_address;
                                    else
                                        bit16_variable_metadata[i].subset_variables[x].subset_variable_address = last_address;
                                else
                                    bit16_variable_metadata[i].subset_variables[x].subset_variable_address = bit16_variable_metadata[i].variable_address + var_id;
                                
                                is_done = true;
                                last_subset_var_id = x;
                                last_was_subset_var = true;
                                last_program_counter = program_counter;
                                last_address = bit16_variable_metadata[i].subset_variables[x].subset_variable_address;
                                
                                break;
                            }
                        }
                        if(is_done) break;
                    }
                }
            }
        }

    public:
        Variables()
            : bit16_length(0), bit32_length(0)
        {}

        ~Variables() = default;
    };
}
#endif