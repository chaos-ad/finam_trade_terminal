#pragma once

class locale_bool 
{
public:
    locale_bool() {}
    locale_bool(bool data) : data(data) {}
    operator bool() const { return data; }
    friend std::ostream & operator << ( std::ostream &out, locale_bool b ) {
        return out << std::boolalpha << b.data;
    }
    friend std::istream & operator >> ( std::istream &in, locale_bool &b ) {
        return in >> std::boolalpha >> b.data;
    }
private:
    bool data;
};
