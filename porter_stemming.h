// An implementation of the Porter Stemming Algorithm
// For a description, see http://tartarus.org/martin/PorterStemmer/
// 
// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include <string>

namespace cdmh {
namespace data_processing {

namespace stemmer {

class stemmer
{
  public:
    std::string operator()(char const *it, char const *ite)
    {
        std::string result(it, ite);
        b = &*result.begin();
        k = (int)result.length()-1;
        j = 0;

        step1ab();
        step1c();
        step2();
        step3();
        step4();
        step5();
        result.resize(k+1);
        return result;
    }

  private:
    // cons(i) is TRUE <=> b[i] is a consonant.
    bool const cons(int i)
    {
        switch (b[i])
        {
            case 'a':
            case 'e':
            case 'i':
            case 'o':
            case 'u':
                return false;

            case 'y':
                return (i == 0) ? true : !cons(i - 1);

            default:
                return true;
        }
    }

    /* cvc(i) is TRUE <=> i-2,i-1,i has the form consonant - vowel - consonant
       and also if the second c is not w,x or y. this is used when trying to
       restore an e at the end of a short word. e.g.

          cav(e), lov(e), hop(e), crim(e), but
          snow, box, tray.

    */
    bool const cvc(int i)
    {
        if (i < 2  ||  !cons(i - 2)  ||  cons(i - 1)  ||  !cons(i))
            return false;

        return (b[i] != 'w'  &&  b[i] != 'x'  &&  b[i] != 'y');
    }


    // double_consonant(j) is TRUE <=> j,(j-1) contain a double consonant
    bool const double_consonant(int j)
    {
        if (j < 1)
            return false;

        if (b[j] != b[j - 1])
            return false;

        return cons(j);
    }

    // ends(s) is true <=> 0,...k ends with the string s.
    bool const ends(char *s)
    {
        int length = s[0];
        if (s[length] != b[k])
            return false;

        if (length > k + 1)
            return false;

        if (memcmp(b + k - length + 1, s + 1, length) != 0)
            return false;

        j = k - length;
        return true;
    }

    /* m() measures the number of consonant sequences between 0 and j. if c is
       a consonant sequence and v a vowel sequence, and <..> indicates arbitrary
       presence,

          <c><v>       gives 0
          <c>vc<v>     gives 1
          <c>vcvc<v>   gives 2
          <c>vcvcvc<v> gives 3
          ....
    */
    int const m()
    {
        int n = 0;
        int i = 0;
        while(true)
        {
            if (i > j)
                return n;

            if (!cons(i))
                break;
            i++;
        }

        i++;
        while(true)
        {
            while(true)
            {
                if (i > j)
                    return n;
                if (cons(i))
                    break;
                i++;
            }
            i++;
            n++;
            while(true)
            {
                if (i > j)
                    return n;
                if (!cons(i))
                    break;
                i++;
            }
            i++;
        }
    }

    // setto(s) sets (j+1),...k to the characters in the string s, readjusting k.
    void setto(char *s)
    {
        int length = s[0];
        memmove(b + j + 1, s + 1, length);
        k = j+length;
    }

    // r(s) is used further down.
    void r(char *s)
    {
        if (m() > 0)
            setto(s);
    }

    // vowelinstem() is TRUE <=> 0,...j contains a vowel
    bool const vowelinstem()
    {
        for (int i = 0; i <= j; i++)
        {
            if (! cons(i))
                return true;
        }

        return false;
    }

    void step1ab()
    {
        // step 1a
        if (b[k] == 's')
        {
            if (ends("\04" "sses"))
                k -= 2;
            else if (ends("\03" "ies"))
                setto("\01" "i");
            else if (b[k - 1] != 's')
                --k;
        }

        // step 1b
        if (ends("\03" "eed"))
        {
            if (m() > 0)
                --k;
        }
        else if ((ends("\02" "ed")  ||  ends("\03" "ing"))  &&  vowelinstem())
        {
            k = j;
            if (ends("\02" "at"))        setto("\03" "ate");
            else if (ends("\02" "bl"))   setto("\03" "ble");
            else if (ends("\02" "iz"))   setto("\03" "ize");      // US english
            else if (ends("\02" "is"))   setto("\03" "ise");      // UK english
            else if (double_consonant(k))
            {
                --k;

                int ch = b[k];
                if (ch == 'l'  ||  ch == 's'  ||  ch == 'z')
                    k++;
            }
            else if (m() == 1  &&  cvc(k))
                setto("\01" "e");
        }
    }

    void step1c()
    {
        if (ends("\01" "y")  &&  vowelinstem())
            b[k] = 'i';
    }

    void step2()
    {
        switch (b[k-1])
        {
           case 'a': if (ends("\07" "ational"))      r("\03" "ate"); 
                     else if (ends("\06" "tional"))  r("\04" "tion");
                     break;
           case 'c': if (ends("\04" "enci"))         r("\04" "ence");
                     else if (ends("\04" "anci"))    r("\04" "ance");
                     break;
           case 'e': if (ends("\04" "izer"))         r("\03" "ize"); 
                     else if (ends("\04" "iser"))    r("\03" "ise");    // UK english
                     break;
           case 'l': if (ends("\03" "bli"))          r("\03" "ble"); 
                     else if (ends("\04" "alli"))    r("\02" "al");  
                     else if (ends("\05" "entli"))   r("\03" "ent"); 
                     else if (ends("\03" "eli"))     r("\01" "e");   
                     else if (ends("\05" "ousli"))   r("\03" "ous"); 
                     break;
           case 'o': if (ends("\07" "ization"))      r("\03" "ize"); 
                     else if (ends("\07" "isation")) r("\03" "ise");    // UK english
                     else if (ends("\05" "ation"))   r("\03" "ate"); 
                     else if (ends("\04" "ator"))    r("\03" "ate"); 
                     break;
           case 's': if (ends("\05" "alism"))        r("\02" "al");  
                     else if (ends("\07" "iveness")) r("\03" "ive"); 
                     else if (ends("\07" "fulness")) r("\03" "ful"); 
                     else if (ends("\07" "ousness")) r("\03" "ous"); 
                     break;
           case 't': if (ends("\05" "aliti"))        r("\02" "al");  
                     else if (ends("\05" "iviti"))   r("\03" "ive"); 
                     else if (ends("\06" "biliti"))  r("\03" "ble"); 
                     break;
           case 'g': if (ends("\04" "logi"))         r("\03" "log"); 
                     break;
        }
    }

    // step3() deals with -ic-, -full, -ness etc. similar strategy to step2.
    void step3()
    {
        switch (b[k])
        {
           case 'e': if (ends("\05" "icate")) { r("\02" "ic"); break; }
                     if (ends("\05" "ative")) { r("\00" "");   break; }
                     if (ends("\05" "alize")) { r("\02" "al"); break; }
                     if (ends("\05" "alise")) { r("\02" "al"); break; } // UK english
                     break;
           case 'i': if (ends("\05" "iciti")) { r("\02" "ic"); break; }
                     break;
           case 'l': if (ends("\04" "ical"))  { r("\02" "ic"); break; }
                     if (ends("\03" "ful"))   { r("\00" "");   break; }
                     break;
           case 's': if (ends("\04" "ness"))  { r("\00" "");   break; }
                     break;
        }
    }

    // step4() takes off -ant, -ence etc., in context <c>vcvc<v>.
    void step4()
    {
        switch (b[k-1])
        {
            case 'a':   if (ends("\02" "al"))       break; return;
            case 'c':   if (ends("\04" "ance"))     break;
                        if (ends("\04" "ence"))     break; return;
            case 'e':   if (ends("\02" "er"))       break; return;
            case 'i':   if (ends("\02" "ic"))       break; return;
            case 'l':   if (ends("\04" "able"))     break;
                        if (ends("\04" "ible"))     break; return;
            case 'n':   if (ends("\03" "ant"))      break;
                        if (ends("\05" "ement"))    break;
                        if (ends("\04" "ment"))     break;
                        if (ends("\03" "ent"))      break; return;
            case 'o':   if (ends("\03" "ion") && j >= 0 && (b[j] == 's' || b[j] == 't')) break;
                        if (ends("\02" "ou"))       break; return; // takes care of -ous
            case 's':   if (ends("\03" "ism"))      break; return;
            case 't':   if (ends("\03" "ate"))      break;
                        if (ends("\03" "iti"))      break; return;
            case 'u':   if (ends("\03" "ous"))      break; return;
            case 'v':   if (ends("\03" "ive"))      break; return;
            case 'z':   if (ends("\03" "ize"))      break; return;
            default:    return;
        }

        if (m() > 1)
            k = j;
    }

    // step5() removes a final -e if m() > 1, and changes -ll to -l if m() > 1.
    void step5()
    {
        j = k;

        // step 5a
        if (b[k] == 'e')
        {
            int a = m();
            if (a > 1  ||  a == 1  &&  !cvc(k - 1))
                --k;
        }

        // step 5b
        if (b[k] == 'l'  &&  double_consonant(k)  &&  m() > 1)
            --k;
    }

  private:
    char *b;       // buffer for word to be stemmed
    int   k;       // offset to the end of the string
    int   j;       // a general offset into the string
};

inline std::string porter_stemmer(char const *it, char const *ite)
{
    stemmer s;
    return s(it, ite);
}

}   // namespace stemmer


template<typename It>
inline
std::string porter_stemmer(It it, It ite)
{
    return stemmer::porter_stemmer(&*it, &*ite);
}

inline
std::string porter_stemmer(char const *string)
{
    return porter_stemmer(string, string + strlen(string));
}

inline
std::string porter_stemmer(std::string const &string)
{
    auto stem = porter_stemmer(string.cbegin(), string.cend());
    return std::string(stem.begin(), stem.end());
}

}   // namespace data_processing
}   // namespace cdmh
