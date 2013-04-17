#include "mysql_priv.h"

extern "C" {
#include "stdint.h"
}
#include "hatoku_cmp.h"


inline TOKU_TYPE mysql_to_toku_type (Field* field) {
    TOKU_TYPE ret_val = toku_type_unknown;
    enum_field_types mysql_type = field->real_type();
    switch (mysql_type) {
    case MYSQL_TYPE_LONG:
    case MYSQL_TYPE_LONGLONG:
    case MYSQL_TYPE_TINY:
    case MYSQL_TYPE_SHORT:
    case MYSQL_TYPE_INT24:
    case MYSQL_TYPE_DATE:
    case MYSQL_TYPE_DATETIME:
    case MYSQL_TYPE_YEAR:
    case MYSQL_TYPE_NEWDATE:
    case MYSQL_TYPE_TIME:
    case MYSQL_TYPE_TIMESTAMP:
    case MYSQL_TYPE_ENUM:
    case MYSQL_TYPE_SET:
        ret_val = toku_type_int;
        goto exit;
    case MYSQL_TYPE_DOUBLE:
        ret_val = toku_type_double;
        goto exit;
    case MYSQL_TYPE_FLOAT:
        ret_val = toku_type_float;
        goto exit;
    case MYSQL_TYPE_NEWDECIMAL:
        ret_val = toku_type_decimal;
        goto exit;
    case MYSQL_TYPE_BIT:
        ret_val = toku_type_bitstream;
        goto exit;
    case MYSQL_TYPE_STRING:
        if (field->binary()) {
            ret_val = toku_type_fixbinary;
        }
        else {
            ret_val = toku_type_fixstring;
        }
        goto exit;
    //
    // I believe these are old types that are no longer
    // in any 5.1 tables, so tokudb does not need
    // to worry about them
    // Putting in this assert in case I am wrong.
    //
    case MYSQL_TYPE_DECIMAL:
    case MYSQL_TYPE_VAR_STRING:
        assert(false);
        goto exit;
    default:
        ret_val = toku_type_unknown;
        goto exit;
    }
exit:
    return ret_val;
}


//
// assuming MySQL in little endian, and we are storing in little endian
//
inline uchar* pack_toku_int (uchar* to_tokudb, uchar* from_mysql, u_int32_t num_bytes) {
    switch (num_bytes) {
    case (1):
    case (2):
    case (3):
    case (4):
    case (8):
        memcpy(to_tokudb, from_mysql, num_bytes);
        break;
    default:
        assert(false);
    }
    return to_tokudb+num_bytes;
}

//
// assuming MySQL in little endian, and we are unpacking to little endian
//
inline uchar* unpack_toku_int(uchar* to_mysql, uchar* from_tokudb, u_int32_t num_bytes) {
    switch (num_bytes) {
    case (1):
    case (2):
    case (3):
    case (4):
    case (8):
        memcpy(to_mysql, from_tokudb, num_bytes);
        break;
    default:
        assert(false);
    }
    return from_tokudb+num_bytes;
}

inline int cmp_toku_int (uchar* a_buf, uchar* b_buf, bool is_unsigned, u_int32_t num_bytes) {
    int ret_val = 0;
    //
    // case for unsigned integers
    //
    if (is_unsigned) {
        u_int32_t a_num, b_num = 0;
        u_int64_t a_big_num, b_big_num = 0;
        switch (num_bytes) {
        case (1):
            a_num = *a_buf;
            b_num = *b_buf;
            ret_val = a_num-b_num;
            goto exit;
        case (2):
            a_num = uint2korr(a_buf);
            b_num = uint2korr(b_buf);
            ret_val = a_num-b_num;
            goto exit;
        case (3):
            a_num = uint3korr(a_buf);
            b_num = uint3korr(b_buf);
            ret_val = a_num-b_num;
            goto exit;
        case (4):
            a_num = uint4korr(a_buf);
            b_num = uint4korr(b_buf);
            if (a_num < b_num) {
                ret_val = -1; goto exit;
            }
            if (a_num > b_num) {
                ret_val = 1; goto exit;
            }
            ret_val = 0;
            goto exit;
        case (8):
            a_big_num = uint8korr(a_buf);
            b_big_num = uint8korr(b_buf);
            if (a_big_num < b_big_num) {
                ret_val = -1; goto exit;
            }
            else if (a_big_num > b_big_num) {
                ret_val = 1; goto exit;
            }
            ret_val = 0;
            goto exit;
        default:
            assert(false);
        }
    }
    //
    // case for signed integers
    //
    else {
        int32_t a_num, b_num = 0;
        int64_t a_big_num, b_big_num = 0;
        switch (num_bytes) {
        case (1):
            a_num = *(signed char *)a_buf;
            b_num = *(signed char *)b_buf;
            ret_val = a_num-b_num;
            goto exit;
        case (2):
            a_num = sint2korr(a_buf);
            b_num = sint2korr(b_buf);
            ret_val = a_num-b_num;
            goto exit;
        case (3):
            a_num = sint3korr(a_buf);
            b_num = sint3korr(b_buf);
            ret_val = a_num - b_num;
            goto exit;
        case (4):
            a_num = sint4korr(a_buf);
            b_num = sint4korr(b_buf);
            if (a_num < b_num) {
                ret_val = -1; goto exit;
            }
            if (a_num > b_num) {
                ret_val = 1; goto exit;
            }
            ret_val = 0;
            goto exit;
        case (8):
            a_big_num = sint8korr(a_buf);
            b_big_num = sint8korr(b_buf);
            if (a_big_num < b_big_num) {
                ret_val = -1; goto exit;
            }
            else if (a_big_num > b_big_num) {
                ret_val = 1; goto exit;
            }
            ret_val = 0;
            goto exit;
        default:
            assert(false);
        }
    }
    //
    // if this is hit, indicates bug in writing of this function
    //
    assert(false);
exit:
    return ret_val;    
}

inline uchar* pack_toku_double (uchar* to_tokudb, uchar* from_mysql) {
    memcpy(to_tokudb, from_mysql, sizeof(double));
    return to_tokudb + sizeof(double);
}


inline uchar* unpack_toku_double(uchar* to_mysql, uchar* from_tokudb) {
    memcpy(to_mysql, from_tokudb, sizeof(double));
    return from_tokudb + sizeof(double);
}

inline int cmp_toku_double(uchar* a_buf, uchar* b_buf) {
    int ret_val;
    double a_num;
    double b_num;
    doubleget(a_num, a_buf);
    doubleget(b_num, b_buf);
    if (a_num < b_num) {
        ret_val = -1;
        goto exit;
    }
    else if (a_num > b_num) {
        ret_val = 1;
        goto exit;
    }
    ret_val = 0;
exit:
    return ret_val;
}


inline uchar* pack_toku_float (uchar* to_tokudb, uchar* from_mysql) {
    memcpy(to_tokudb, from_mysql, sizeof(float));
    return to_tokudb + sizeof(float);
}


inline uchar* unpack_toku_float(uchar* to_mysql, uchar* from_tokudb) {
    memcpy(to_mysql, from_tokudb, sizeof(float));
    return from_tokudb + sizeof(float);
}

inline int cmp_toku_float(uchar* a_buf, uchar* b_buf) {
    int ret_val;
    float a_num;
    float b_num;
    //
    // This is the way Field_float::cmp gets the floats from the buffers
    //
    memcpy(&a_num, a_buf, sizeof(float));
    memcpy(&b_num, b_buf, sizeof(float));
    if (a_num < b_num) {
        ret_val = -1;
        goto exit;
    }
    else if (a_num > b_num) {
        ret_val = 1;
        goto exit;
    }
    ret_val = 0;
exit:
    return ret_val;
}


inline uchar* pack_toku_binary(uchar* to_tokudb, uchar* from_mysql, u_int32_t num_bytes) {
    memcpy(to_tokudb, from_mysql, num_bytes);
    return to_tokudb + num_bytes;
}

inline uchar* unpack_toku_binary(uchar* to_mysql, uchar* from_tokudb, u_int32_t num_bytes) {
    memcpy(to_mysql, from_tokudb, num_bytes);
    return from_tokudb + num_bytes;
}


inline int cmp_toku_binary(
    uchar* a_buf, 
    u_int32_t a_num_bytes, 
    uchar* b_buf, 
    u_int32_t b_num_bytes
    ) 
{
    int ret_val = 0;
    u_int32_t num_bytes_to_cmp = (a_num_bytes < b_num_bytes) ? a_num_bytes : b_num_bytes;
    ret_val = memcmp(a_buf, b_buf, num_bytes_to_cmp);
    if ((ret_val != 0) || (a_num_bytes == b_num_bytes)) {
        goto exit;
    }
    if (a_num_bytes < b_num_bytes) {
        ret_val = -1;
        goto exit;
    }
    else {
        ret_val = 1;
        goto exit;
    }
exit:
    return ret_val;
}


inline int cmp_toku_string(
    uchar* a_buf,
    u_int32_t a_num_bytes,
    uchar* b_buf, 
    u_int32_t b_num_bytes,
    u_int32_t charset_number
    ) 
{
    int ret_val = 0;
    CHARSET_INFO* charset = NULL;
    charset = get_charset(charset_number, MYF(MY_WME));
    ret_val = charset->coll->strnncollsp(
        charset,
        a_buf, 
        a_num_bytes,
        b_buf, 
        b_num_bytes, 
        0
        );
    return ret_val;
}

int compare_field(
    uchar* a_buf, 
    uchar* b_buf, 
    Field* field,
    u_int32_t key_part_length, //I really hope this is temporary as I phase out the pack_cmp stuff
    u_int32_t* a_bytes_read, 
    u_int32_t* b_bytes_read
    ) {
    int ret_val = 0;
    TOKU_TYPE toku_type = mysql_to_toku_type(field);
    u_int32_t num_bytes = 0;
    switch(toku_type) {
    case (toku_type_int):
        ret_val = cmp_toku_int(
            a_buf, 
            b_buf, 
            field->flags & UNSIGNED_FLAG, 
            field->pack_length()
            );
        *a_bytes_read = field->pack_length();
        *b_bytes_read = field->pack_length();
        goto exit;
    case (toku_type_double):
        assert(field->pack_length() == sizeof(double));
        assert(key_part_length == sizeof(double));
        ret_val = cmp_toku_double(a_buf, b_buf);
        *a_bytes_read = sizeof(double);
        *b_bytes_read = sizeof(double);
        goto exit;
    case (toku_type_decimal):
    case (toku_type_bitstream):
    case (toku_type_fixbinary):
        num_bytes = field->pack_length();
        set_if_smaller(num_bytes, key_part_length);
        ret_val = cmp_toku_binary(a_buf, num_bytes, b_buf,num_bytes);
        *a_bytes_read = num_bytes;
        *b_bytes_read = num_bytes;
        goto exit;
    case (toku_type_fixstring):
        num_bytes = field->pack_length();
        set_if_smaller(num_bytes, key_part_length);
        ret_val = cmp_toku_string(a_buf, num_bytes, b_buf,num_bytes, field->charset()->number);
        *a_bytes_read = num_bytes;
        *b_bytes_read = num_bytes;
        goto exit;
    default:
        *a_bytes_read = field->packed_col_length(a_buf, key_part_length);
        *b_bytes_read = field->packed_col_length(b_buf, key_part_length);
        ret_val = field->pack_cmp(a_buf, b_buf, key_part_length, 0);
        goto exit;
    }
    assert(false);
exit:
    return ret_val;
}


//
// at the moment, this returns new position in buffer
// I want to change this to be num_bytes_packed
// cannot do it until all functions converted, because until
// then, still relying on field->pack_cmp
//
uchar* pack_field(
    uchar* to_tokudb,
    uchar* from_mysql,
    Field* field,
    u_int32_t key_part_length //I really hope this is temporary as I phase out the pack_cmp stuff
    )
{
    uchar* new_pos = NULL;
    u_int32_t num_bytes = 0;
    TOKU_TYPE toku_type = mysql_to_toku_type(field);
    switch(toku_type) {
    case (toku_type_int):
        assert(key_part_length == field->pack_length());
        new_pos = pack_toku_int(
            to_tokudb, 
            from_mysql,
            field->pack_length()
            );
        goto exit; 
    case (toku_type_double):
        assert(field->pack_length() == sizeof(double));
        assert(key_part_length == sizeof(double));
        new_pos = pack_toku_double(to_tokudb, from_mysql);
        goto exit;
    case (toku_type_float):
        assert(field->pack_length() == sizeof(float));
        assert(key_part_length == sizeof(float));
        new_pos = pack_toku_float(to_tokudb, from_mysql);
        goto exit;
    case (toku_type_decimal):
    case (toku_type_bitstream):
    case (toku_type_fixbinary):
    case (toku_type_fixstring):
        num_bytes = field->pack_length();
        set_if_smaller(num_bytes, key_part_length);
        new_pos = pack_toku_binary(
            to_tokudb,
            from_mysql,
            num_bytes
            );
        goto exit;
    default:
        assert(toku_type == toku_type_unknown);
        new_pos = field->pack_key(
            to_tokudb, 
            from_mysql,
            key_part_length, 
            TRUE
            );
        goto exit;
    }
    assert(false);
exit:
    return new_pos;
}

uchar* pack_key_field(
    uchar* to_tokudb,
    uchar* from_mysql,
    Field* field,
    u_int32_t key_part_length //I really hope this is temporary as I phase out the pack_cmp stuff
    )
{
    uchar* new_pos = NULL;
    TOKU_TYPE toku_type = mysql_to_toku_type(field);
    switch(toku_type) {
    case (toku_type_int):
    case (toku_type_double):
    case (toku_type_float):
    case (toku_type_decimal):
    case (toku_type_bitstream):
    case (toku_type_fixbinary):
    case (toku_type_fixstring):
        new_pos = pack_field(to_tokudb, from_mysql, field, key_part_length);
        goto exit;
    default:
        assert(toku_type == toku_type_unknown);
        new_pos= field->pack_key_from_key_image(
            to_tokudb, 
            from_mysql,
            key_part_length, 
            true
            );
        goto exit;
    }
    assert(false);
exit:
    return new_pos;
}


uchar* unpack_field(
    uchar* to_mysql,
    uchar* from_tokudb,
    Field* field,
    u_int32_t key_part_length
    )
{
    uchar* new_pos = NULL;
    u_int32_t num_bytes = 0;
    TOKU_TYPE toku_type = mysql_to_toku_type(field);
    switch(toku_type) {
    case (toku_type_int):
        assert(key_part_length == field->pack_length());
        new_pos = unpack_toku_int(
            to_mysql,
            from_tokudb,
            field->pack_length()
            );
        goto exit;    
    case (toku_type_double):
        assert(field->pack_length() == sizeof(double));
        assert(key_part_length == sizeof(double));
        new_pos = unpack_toku_double(to_mysql, from_tokudb);
        goto exit;
    case (toku_type_float):
        assert(field->pack_length() == sizeof(float));
        assert(key_part_length == sizeof(float));
        new_pos = unpack_toku_float(to_mysql, from_tokudb);
        goto exit;
    case (toku_type_decimal):
    case (toku_type_bitstream):
    case (toku_type_fixbinary):
    case (toku_type_fixstring):
        num_bytes = field->pack_length();
        set_if_smaller(num_bytes, key_part_length);
        new_pos = unpack_toku_binary(
            to_mysql,
            from_tokudb,
            num_bytes
            );
        goto exit;
    default:
        assert(toku_type == toku_type_unknown);
        new_pos = (uchar *) field->unpack_key(
            to_mysql, 
            from_tokudb,
            key_part_length, 
            TRUE
            );
        goto exit;
    }
    assert(false);
exit:
    return new_pos;
}


inline int tokudb_compare_two_hidden_keys(
    const void* new_key_data, 
    const u_int32_t new_key_size, 
    const void*  saved_key_data,
    const u_int32_t saved_key_size
    ) {
    assert( (new_key_size >= TOKUDB_HIDDEN_PRIMARY_KEY_LENGTH) && (saved_key_size >= TOKUDB_HIDDEN_PRIMARY_KEY_LENGTH) );
    ulonglong a = hpk_char_to_num((uchar *) new_key_data);
    ulonglong b = hpk_char_to_num((uchar *) saved_key_data);
    return a < b ? -1 : (a > b ? 1 : 0);
}

int tokudb_cmp_hidden_key(DB * file, const DBT * new_key, const DBT * saved_key) {
    return tokudb_compare_two_hidden_keys(
        new_key->data, 
        new_key->size,
        saved_key->data,
        saved_key->size
        );
}

int tokudb_compare_two_keys(
    KEY *key, 
    const void* new_key_data, 
    const u_int32_t new_key_size, 
    const void*  saved_key_data,
    const u_int32_t saved_key_size,
    bool cmp_prefix
    ) {
    uchar new_key_inf_val = *(uchar *) new_key_data;
    uchar saved_key_inf_val = *(uchar *) saved_key_data;
    //
    // first byte is "infinity" byte
    //
    uchar *new_key_ptr = (uchar *)(new_key_data) + 1;
    uchar *saved_key_ptr = (uchar *)(saved_key_data) + 1;
    KEY_PART_INFO *key_part = key->key_part, *end = key_part + key->key_parts;
    int ret_val;
    //
    // do not include the inf val at the beginning
    //
    uint new_key_length = new_key_size - sizeof(uchar);
    uint saved_key_length = saved_key_size - sizeof(uchar);

    //DBUG_DUMP("key_in_index", saved_key_ptr, saved_key->size);
    for (; key_part != end && (int) new_key_length > 0 && (int) saved_key_length > 0; key_part++) {
        int cmp;
        uint new_key_field_length;
        uint saved_key_field_length;
        if (key_part->field->null_bit) {
            assert(new_key_ptr   < (uchar *) new_key_data   + new_key_size);
            assert(saved_key_ptr < (uchar *) saved_key_data + saved_key_size);
            if (*new_key_ptr != *saved_key_ptr) {
                return ((int) *new_key_ptr - (int) *saved_key_ptr); }
            saved_key_ptr++;
            new_key_length--;
            saved_key_length--;
            if (!*new_key_ptr++) { continue; }
        }
        cmp = compare_field(
            new_key_ptr, 
            saved_key_ptr,
            key_part->field,
            key_part->length,
            &new_key_field_length,
            &saved_key_field_length
            );
        if (cmp) {
            return cmp;
        }

        assert(new_key_length >= new_key_field_length);
        assert(saved_key_length >= saved_key_field_length);

        new_key_ptr      += new_key_field_length;
        new_key_length   -= new_key_field_length;
        saved_key_ptr    += saved_key_field_length;
        saved_key_length -= saved_key_field_length;
    }
    if (cmp_prefix || (new_key_length == 0 && saved_key_length == 0) ) {
        ret_val = 0;
    }
    //
    // at this point, one SHOULD be 0
    //
    else if (new_key_length == 0 && saved_key_length > 0) {
        ret_val = (new_key_inf_val == COL_POS_INF ) ? 1 : -1; 
    }
    else if (new_key_length > 0 && saved_key_length == 0) {
        ret_val = (saved_key_inf_val == COL_POS_INF ) ? -1 : 1; 
    }
    //
    // this should never happen, perhaps we should assert(false)
    //
    else {
        ret_val = new_key_length - saved_key_length;
    }
    return ret_val;
}



//
// this is super super ugly, copied from compare_two_keys so that it can get done fast
//
int tokudb_compare_two_clustered_keys(KEY *key, KEY* primary_key, const DBT * new_key, const DBT * saved_key) {
    uchar new_key_inf_val = *(uchar *) new_key->data;
    uchar saved_key_inf_val = *(uchar *) saved_key->data;
    //
    // first byte is "infinity" byte
    //
    uchar *new_key_ptr = (uchar *)(new_key->data) + 1;
    uchar *saved_key_ptr = (uchar *)(saved_key->data) + 1;
    KEY_PART_INFO *key_part = key->key_part, *end = key_part + key->key_parts;
    int ret_val;
    //
    // do not include the inf val at the beginning
    //
    uint new_key_length = new_key->size - sizeof(uchar);
    uint saved_key_length = saved_key->size - sizeof(uchar);

    //DBUG_DUMP("key_in_index", saved_key_ptr, saved_key->size);
    for (; key_part != end && (int) new_key_length > 0 && (int) saved_key_length > 0; key_part++) {
        int cmp;
        uint new_key_field_length;
        uint saved_key_field_length;
        if (key_part->field->null_bit) {
            assert(new_key_ptr   < (uchar *) new_key->data   + new_key->size);
            assert(saved_key_ptr < (uchar *) saved_key->data + saved_key->size);
            if (*new_key_ptr != *saved_key_ptr) {
                return ((int) *new_key_ptr - (int) *saved_key_ptr); }
            saved_key_ptr++;
            new_key_length--;
            saved_key_length--;
            if (!*new_key_ptr++) { continue; }
        }
        cmp = compare_field(
            new_key_ptr, 
            saved_key_ptr,
            key_part->field,
            key_part->length,
            &new_key_field_length,
            &saved_key_field_length
            );
        if (cmp) {
            return cmp;
        }

        assert(new_key_length >= new_key_field_length);
        assert(saved_key_length >= saved_key_field_length);

        new_key_ptr      += new_key_field_length;
        new_key_length   -= new_key_field_length;
        saved_key_ptr    += saved_key_field_length;
        saved_key_length -= saved_key_field_length;
    }
    if (new_key_length == 0 && saved_key_length == 0){
        ret_val = 0;
    }
    else if (new_key_length == 0 && saved_key_length > 0) {
        ret_val = (new_key_inf_val == COL_POS_INF ) ? 1 : -1; 
    }
    else if (new_key_length > 0 && saved_key_length == 0) {
        ret_val = (saved_key_inf_val == COL_POS_INF ) ? -1 : 1; 
    }
    //
    // now we compare the primary key
    //
    else {
        if (primary_key == NULL) {
            //
            // primary key hidden
            //
            ulonglong a = hpk_char_to_num((uchar *) new_key_ptr);
            ulonglong b = hpk_char_to_num((uchar *) saved_key_ptr);
            ret_val =  a < b ? -1 : (a > b ? 1 : 0);
        }
        else {
            //
            // primary key not hidden, I know this is bad, basically copying the code from above
            //
            key_part = primary_key->key_part;
            end = key_part + primary_key->key_parts;
            for (; key_part != end && (int) new_key_length > 0 && (int) saved_key_length > 0; key_part++) {
                int cmp;
                uint new_key_field_length;
                uint saved_key_field_length;
                if (key_part->field->null_bit) {
                    assert(new_key_ptr   < (uchar *) new_key->data   + new_key->size);
                    assert(saved_key_ptr < (uchar *) saved_key->data + saved_key->size);
                    if (*new_key_ptr != *saved_key_ptr) {
                        return ((int) *new_key_ptr - (int) *saved_key_ptr); }
                    saved_key_ptr++;
                    new_key_length--;
                    saved_key_length--;
                    if (!*new_key_ptr++) { continue; }
                }
                cmp = compare_field(
                    new_key_ptr, 
                    saved_key_ptr,
                    key_part->field,
                    key_part->length,
                    &new_key_field_length,
                    &saved_key_field_length
                    );
                if (cmp) {
                    return cmp;
                }
                
                assert(new_key_length >= new_key_field_length);
                assert(saved_key_length >= saved_key_field_length);
                
                new_key_ptr      += new_key_field_length;
                new_key_length   -= new_key_field_length;
                saved_key_ptr    += saved_key_field_length;
                saved_key_length -= saved_key_field_length;
            }
            //
            // at this point, we have compared the actual keys and the primary key, we return 0
            //
            ret_val = 0;
        }
    }
    return ret_val;
}


int tokudb_cmp_packed_key(DB *file, const DBT *keya, const DBT *keyb) {
    assert(file->app_private != 0);
    KEY *key = (KEY *) file->app_private;
    KEY *primary_key = (KEY *) file->api_internal;
    if (key->flags & HA_CLUSTERING) {
        return tokudb_compare_two_clustered_keys(key, primary_key, keya, keyb);
    }
    return tokudb_compare_two_keys(key, keya->data, keya->size, keyb->data, keyb->size, false);
}

int tokudb_cmp_primary_key(DB *file, const DBT *keya, const DBT *keyb) {
    assert(file->app_private != 0);
    KEY *key = (KEY *) file->api_internal;
    return tokudb_compare_two_keys(key, keya->data, keya->size, keyb->data, keyb->size, false);
}

//TODO: QQQ Only do one direction for prefix.
int tokudb_prefix_cmp_packed_key(DB *file, const DBT *keya, const DBT *keyb) {
    assert(file->app_private != 0);
    KEY *key = (KEY *) file->app_private;
    return tokudb_compare_two_keys(key, keya->data, keya->size, keyb->data, keyb->size, true);
}
