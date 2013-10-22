// An implementation of the Porter Stemming Algorithm
// For a description, see http://tartarus.org/martin/PorterStemmer/
// 
// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <string>
#include <cassert>
#include "string_view.h"

namespace cdmh {

namespace data_processing {

namespace porter_stemmer {

class stemmer
{
  public:
    std::string operator()(char const *it, char const *ite)
    {
        std::string result(it, ite);
        word = &*result.begin();
        last_pos = (int)result.length()-1;
        offset = 0;

        step1a();
        step1b();
        step1c();
        step2();
        step3();
        step4();
        step5a();
        step5b();
        result.resize(last_pos+1);
        return result;
    }

  private:
    // is_consonant(i) is TRUE <=> word[i] is a consonant.
    bool const is_consonant(int i) const
    {
        switch (word[i])
        {
            case 'a':  case 'A':
            case 'e':  case 'E':
            case 'i':  case 'I':
            case 'o':  case 'O':
            case 'u':  case 'U':
                return false;

            case 'y':  case 'Y':
                return (i == 0) ? true : !is_consonant(i - 1);

            default:
                return true;
        }
    }

    /* is_consonant_vowel_consonant(i) is TRUE <=> i-2,i-1,i has the form consonant - vowel - consonant
       and also if the second c is not w,x or y. this is used when trying to
       restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

    */
    bool const is_consonant_vowel_consonant(int i) const
    {
        if (i < 2  ||  !is_consonant(i - 2)  ||  is_consonant(i - 1)  ||  !is_consonant(i))
            return false;

        return word[i] != 'w'  &&  word[i] != 'x'  &&  word[i] != 'y'
            && word[i] != 'W'  &&  word[i] != 'X'  &&  word[i] != 'Y';
    }


    // double_consonant(offset) is TRUE <=> offset,(offset-1) contain a double consonant
    bool const double_consonant(int offset)
    {
        if (offset < 1)
            return false;

        if (lower(word[offset]) != lower(word[offset - 1]))
            return false;

        return is_consonant(offset);
    }

    // ends(s) is true <=> 0,...last_pos ends with the string s.
    bool const ends(int length, char *s)
    {
        assert(length == strlen(s));

        if (lower(s[length-1]) != lower(word[last_pos]))
            return false;
        else if (length > last_pos + 1)
            return false;
        else if (strncasecmp(word + last_pos - length + 1, s, length) != 0)
            return false;

        offset = last_pos - length;
        return true;
    }

    char lower(char ch) const
    {
        if (ch >= 'A'  &&  ch <= 'Z')
            ch += 'a' - 'A';
        return ch;
    }

    /* measure() measures the number of consonant sequences between 0 and offset. if c is
       a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
       presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc<v> gives 3
          ....
    */
    int const measure() const
    {
        int i = 0;
        while (i <= offset  &&  is_consonant(i))
            ++i;
        if (i > offset)
            return 0;
        assert(!is_consonant(i));

        int count = 0;
        while (true)
        {
            while (i <= offset  &&  !is_consonant(i))
                ++i;
            if (i > offset)
                return count;
            assert(is_consonant(i));

            ++count;
            while (i <= offset  &&  is_consonant(i))
                ++i;
            if (i > offset)
                return count;
            assert(!is_consonant(i));
            ++i;
        }
    }

    // setto(s) sets (offset+1),...last_pos to the characters in the string s, re-adjusting last_pos.
    void setto(int length, char *s)
    {
        assert(length == strlen(s));
        memmove(word + offset + 1, s, length);
        last_pos = offset + length;
    }

    void replace_if_measure(int length, char *s)
    {
        if (measure() > 0)
            setto(length, s);
    }

    // vowelinstem() is TRUE <=> 0,...offset contains a vowel
    bool const vowelinstem() const
    {
        for (int i = 0; i <= offset; ++i)
        {
            if (!is_consonant(i))
                return true;
        }

        return false;
    }

    void step1a()
    {
        if (lower(word[last_pos]) == 's')
        {
            if (ends(4, "sses"))
                last_pos -= 2;
            else if (ends(3, "ies"))
                setto(1, "i");
            else if (lower(word[last_pos - 1]) != 's')
                --last_pos;
        }
    }

    void step1b()
    {
        if (ends(3, "eed"))
        {
            if (measure() > 0)
                --last_pos;
        }
        else if ((ends(2, "ed")  ||  ends(3, "ing"))  &&  vowelinstem())
        {
            last_pos = offset;
            if (ends(2, "at"))        setto(3, "ate");
            else if (ends(2, "bl"))   setto(3, "ble");
            else if (ends(2, "iz"))   setto(3, "ize");      // US english
            else if (ends(2, "is"))   setto(3, "ise");      // UK english
            else if (double_consonant(last_pos))
            {
                --last_pos;

                int ch = word[last_pos];
                if (ch == 'l'  ||  ch == 's'  ||  ch == 'z'  ||  ch == 'L'  ||  ch == 'S'  ||  ch == 'Z')
                    last_pos++;
            }
            else if (measure() == 1  &&  is_consonant_vowel_consonant(last_pos))
                setto(1, "e");
        }
    }

    void step1c()
    {
        if (ends(1, "y")  &&  vowelinstem())
            word[last_pos] = 'i';
    }

    void step2()
    {
        switch (lower(word[last_pos-1]))
        {
           case 'a':    if (ends(7, "ational"))         replace_if_measure(3, "ate"); 
                        else if (ends(6, "tional"))     replace_if_measure(4, "tion");
                        break;
           case 'c':    if (ends(4, "enci"))            replace_if_measure(4, "ence");
                        else if (ends(4, "anci"))       replace_if_measure(4, "ance");
                        break;
           case 'e':    if (ends(4, "izer"))            replace_if_measure(3, "ize"); 
                        else if (ends(4, "iser"))       replace_if_measure(3, "ise");    // UK english
                        break;
           case 'l':    if (ends(3, "bli"))             replace_if_measure(3, "ble"); 
                        else if (ends(4, "alli"))       replace_if_measure(2, "al");  
                        else if (ends(5, "entli"))      replace_if_measure(3, "ent"); 
                        else if (ends(3, "eli"))        replace_if_measure(1, "e");   
                        else if (ends(5, "ousli"))      replace_if_measure(3, "ous"); 
                        break;
           case 'o':    if (ends(7, "ization"))         replace_if_measure(3, "ize"); 
                        else if (ends(7, "isation"))    replace_if_measure(3, "ise");    // UK english
                        else if (ends(5, "ation"))      replace_if_measure(3, "ate"); 
                        else if (ends(4, "ator"))       replace_if_measure(3, "ate"); 
                        break;
           case 's':    if (ends(5, "alism"))           replace_if_measure(2, "al");  
                        else if (ends(7, "iveness"))    replace_if_measure(3, "ive"); 
                        else if (ends(7, "fulness"))    replace_if_measure(3, "ful"); 
                        else if (ends(7, "ousness"))    replace_if_measure(3, "ous"); 
                        break;
           case 't':    if (ends(5, "aliti"))           replace_if_measure(2, "al");  
                        else if (ends(5, "iviti"))      replace_if_measure(3, "ive"); 
                        else if (ends(6, "biliti"))     replace_if_measure(3, "ble"); 
                        break;
           case 'g':    if (ends(4, "logi"))            replace_if_measure(3, "log"); 
                        break;
        }
    }

    // step3() deals with -ic-, -full, -ness etc. similar strategy to step2.
    void step3()
    {
        switch (lower(word[last_pos]))
        {
           case 'e':    if (ends(5, "icate"))       replace_if_measure(2, "ic");
                        else if (ends(5, "ative"))  replace_if_measure(0, "");  
                        else if (ends(5, "alize"))  replace_if_measure(2, "al");
                        else if (ends(5, "alise"))  replace_if_measure(2, "al");    // UK english
                        break;
           case 'i':    if (ends(5, "iciti"))       replace_if_measure(2, "ic");
                        break;
           case 'l':    if (ends(4, "ical"))        replace_if_measure(2, "ic");
                        else if (ends(3, "ful"))    replace_if_measure(0, "");
                        break;
           case 's':    if (ends(4, "ness"))        replace_if_measure(0, "");
                        break;
        }
    }

    // step4() takes off -ant, -ence etc., in context <c>vcvc<v>.
    void step4()
    {
        switch (lower(word[last_pos-1]))
        {
            case 'a':   if (ends(2, "al"))          break; else return;
            case 'c':   if (ends(4, "ance"))        break;
                        else if (ends(4, "ence"))   break; else return;
            case 'e':   if (ends(2, "er"))          break; else return;
            case 'i':   if (ends(2, "ic"))          break; else return;
            case 'l':   if (ends(4, "able"))        break;
                        else if (ends(4, "ible"))   break; else return;
            case 'n':   if (ends(3, "ant"))         break;
                        else if (ends(5, "ement"))  break;
                        else if (ends(4, "ment"))   break;
                        else if (ends(3, "ent"))    break; else return;
            case 'o':   if (ends(3, "ion") && offset >= 0 && (word[offset] == 's' || word[offset] == 't'  ||  word[offset] == 'S' || word[offset] == 'T')) break;
                        else if (ends(2, "ou"))     break; else return; // takes care of -ous
            case 's':   if (ends(3, "ism"))         break; else return;
            case 't':   if (ends(3, "ate"))         break;
                        else if (ends(3, "iti"))    break; else return;
            case 'u':   if (ends(3, "ous"))         break; else return;
            case 'v':   if (ends(3, "ive"))         break; else return;
            case 'z':   if (ends(3, "ize"))         break; else return;
            default:    return;
        }

        if (measure() > 1)
            last_pos = offset;
    }

    // remove a final -e if measure() > 1,
    void step5a()
    {
        offset = last_pos;

        // step 5a
        if (lower(word[last_pos]) == 'e')
        {
            int a = measure();
            if (a > 1  ||  a == 1  &&  !is_consonant_vowel_consonant(last_pos - 1))
                --last_pos;
        }
    }

    // change -ll to -l if measure() > 1.
    void step5b()
    {
        if ((word[last_pos] == 'l'  ||  word[last_pos] == 'L')  &&  double_consonant(last_pos)  &&  measure() > 1)
            --last_pos;
    }

  private:
    char *word;     // buffer for word to be stemmed
    int   last_pos; // offset to the end of the string (not one-past the end!)
    int   offset;   // a general offset into the string
};

inline std::string stem(char const *it, char const *ite)
{
    porter_stemmer::stemmer s;
    return s(it, ite);
}

template<typename It>
inline
std::string stem(It it, It ite)
{
    return stem(&*it, &*ite);
}

inline
std::string stem(char const *string)
{
    return stem(string, string + strlen(string));
}

inline
std::string stem(std::string const &string)
{
    return stem(string.cbegin(), string.cend());
}

}   // namespace porter_stemmer


}   // namespace data_processing

}   // namespace cdmh
