#include "headers/svlp_filter/svlp_filter.h"

#ifndef UINT8_MAX
#define UINT8_MAX 255
#endif

svlp::Simple_filter::Simple_filter()
    : type(invalid)
    , list()
    , list_length(0)
{
}

svlp::Simple_filter::Simple_filter(svlp::Simple_filter::Type type)
    : type(type)
    , list()
    , list_length(0)
{
}

svlp::Simple_filter::uid_t svlp::Simple_filter::insert(svlp::Simple_filter::Item& item)
{
    if (list_length == sizeof(list) / sizeof(list[0]))
    {
        return UINT8_MAX;
    }
    list[list_length] = item;
    return static_cast<uid_t>(list_length++);
}

bool svlp::Simple_filter::remove(const uid_t uid)
{
    if (uid < list_length)
    {
        for (uid_t i = uid; i < list_length; i++)
        {
            list[uid] = list[uid + 1];
        }
        return true;
    }
    return false;
}

bool svlp::Simple_filter::is_list_contain(uint8_t infocode, uint8_t msgcode) const
{
    for (size_t i = 0; i < list_length; i++)
    {
        if (list[i].info_code == infocode && list[i].msg_code == msgcode)
            return true;
    }
    return false;
}

svlp::Filter_model::Type svlp::Simple_filter::getType() const
{
    return type;
}

void svlp::Simple_filter::setType(const svlp::Filter_model::Type& value)
{
    type = value;
}

bool svlp::Simple_filter::check(uint8_t infocode, uint8_t msg_code) const
{
    bool result = false;

    switch (type)
    {
    case blacklist:
        result = !is_list_contain(infocode, msg_code);
        break;
    case whitelist:
        result = is_list_contain(infocode, msg_code);
        break;

    case invalid:
        result = true;
    }
    return result;
}

size_t svlp::Simple_filter::length() const
{
    return list_length;
}

void svlp::Simple_filter::clear()
{
    list_length = 0;
}

svlp::Simple_filter::Item svlp::Simple_filter::at(size_t index) const
{
    if (index < list_length)
    {
        return list[index];
    }
}

svlp::Filter_model::~Filter_model() {}
