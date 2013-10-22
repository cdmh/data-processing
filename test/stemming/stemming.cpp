// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include"stdafx.h"
#include"porter_stemming.h"

#define CATCH_CONFIG_RUNNER
#include"catch.hpp"

namespace { // anonymous namespace

using cdmh::data_processing::porter_stemmer::stem;

TEST_CASE("stemmer/Step1a","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("caress") == "caress");
    CHECK(stem("caresses") == "caress");
    CHECK(stem("ponies") == "poni");
    CHECK(stem("ties") == "ti");
    CHECK(stem("cats") == "cat");
}

TEST_CASE("stemmer/Step1b","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("conflated") == "conflate");
    CHECK(stem("feed") == "feed");
    CHECK(stem("agreed") == "agree");
    CHECK(stem("plastered") == "plaster");
    CHECK(stem("bled") == "bled");
    CHECK(stem("motoring") == "motor");
    CHECK(stem("sing") == "sing");
    CHECK(stem("troubled") == "trouble");
    CHECK(stem("sized") == "size");
    CHECK(stem("hopping") == "hop");
    CHECK(stem("tanned") == "tan");
    CHECK(stem("falling") == "fall");
    CHECK(stem("hissing") == "hiss");
    CHECK(stem("fizzed") == "fizz");
    CHECK(stem("failing") == "fail");
    CHECK(stem("filing") == "file");
    CHECK(stem("disabled") == "disable");
    CHECK(stem("matting") == "mat");
    CHECK(stem("mating") == "mate");
    CHECK(stem("meeting") == "meet");
    CHECK(stem("milling") == "mill");
    CHECK(stem("messing") == "mess");
    CHECK(stem("meetings") == "meet");
}

TEST_CASE("stemmer/step1c","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("happy") == "happi");
    CHECK(stem("sky") == "sky");
}

TEST_CASE("stemmer/step2","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("relational") == "relate");
    CHECK(stem("conditional") == "condition");
    CHECK(stem("rational") == "rational");
    CHECK(stem("valenci") == "valence");
    CHECK(stem("hesitanci") == "hesitance");
    CHECK(stem("digitizer") == "digitize");
    CHECK(stem("digitiser") == "digitise");
    CHECK(stem("conformabli") == "conformable");
    CHECK(stem("radicalli") == "radical");
    CHECK(stem("differentli") == "different");
    CHECK(stem("vileli") == "vile");
    CHECK(stem("analogousli") == "analogous");
    CHECK(stem("vietnamization") == "vietnamize");
    CHECK(stem("vietnamisation") == "vietnamise");
    CHECK(stem("predication") == "predicate");
    CHECK(stem("operator") == "operate");
    CHECK(stem("feudalism") == "feudal");
    CHECK(stem("decisiveness") == "decisive");
    CHECK(stem("hopefulness") == "hopeful");
    CHECK(stem("callousness") == "callous");
    CHECK(stem("formaliti") == "formal");
    CHECK(stem("sensitiviti") == "sensitive");
    CHECK(stem("sensibiliti") == "sensible");
}

TEST_CASE("stemmer/step3","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("triplicate") == "triplic");
    CHECK(stem("formative") == "form");
    CHECK(stem("formalize") == "formal");
    CHECK(stem("formalise") == "formal");
    CHECK(stem("electriciti") == "electric");
    CHECK(stem("electrical") == "electric");
    CHECK(stem("hopeful") == "hope");
    CHECK(stem("goodness") == "good");
}

TEST_CASE("stemmer/step4","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("revival") == "reviv");
    CHECK(stem("allowance") == "allow");
    CHECK(stem("inference") == "infer");
    CHECK(stem("airliner") == "airlin");
    CHECK(stem("gyroscopic") == "gyroscop");
    CHECK(stem("adjustable") == "adjust");
    CHECK(stem("defensible") == "defens");
    CHECK(stem("irritant") == "irrit");
    CHECK(stem("replacement") == "replac");
    CHECK(stem("adjustment") == "adjust");
    CHECK(stem("dependent") == "depend");
    CHECK(stem("adoption") == "adopt");
    CHECK(stem("homologou") == "homolog");
    CHECK(stem("communism") == "commun");
    CHECK(stem("activate") == "activ");
    CHECK(stem("angulariti") == "angular");
    CHECK(stem("homologous") == "homolog");
    CHECK(stem("effective") == "effect");
    CHECK(stem("bowdlerize") == "bowdler");
    CHECK(stem("bowdlerise") == "bowdler");
}

TEST_CASE("stemmer/step5a","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("probate") == "probat");
    CHECK(stem("rate") == "rate");
    CHECK(stem("cease") == "ceas");
}

TEST_CASE("stemmer/step5b","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("controll") == "control");
    CHECK(stem("roll") == "roll");
}

TEST_CASE("stemmer/stem too short","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(stem("relate") == "relate");
    CHECK(stem("probate") == "probate");
    CHECK(stem("conflate") == "conflate");
    CHECK(stem("pirate") == "pirate");
    CHECK(stem("prelate") == "prelate");

    CHECK(stem("derivate") == "deriv");
    CHECK(stem("activate") == "activ");
    CHECK(stem("demonstrate") == "demonstr");
    CHECK(stem("necessitate") == "necessit");
    CHECK(stem("renovate") == "renov");

    CHECK(stem("derivate") == stem("derive"));
    CHECK(stem("activate") == stem("active"));
    CHECK(stem("demonstrate") == stem("demonstrable"));
    CHECK(stem("necessitate") == stem("necessitous"));
}

TEST_CASE("stemmer/misc","miscellaneous and common errors")
{
    CHECK(stem("ion") == "ion");
    CHECK(stem("possibly") == stem("possible"));
    CHECK(stem("archaeology") == stem("archaeological"));

    // test common errors
    CHECK(stem("argument") == "argum");
}

TEST_CASE("stemmer/case match","")
{
    CHECK(cdmh::strcasecmp(stem("iOn").c_str(), "ion") == 0);
    CHECK(cdmh::strcasecmp(stem("possiBLy").c_str(), stem("possiblE").c_str()) == 0);
    CHECK(cdmh::strcasecmp(stem("ARCHAEOLOGY").c_str(), stem("archaeological").c_str()) == 0);
}

TEST_CASE("stemmer/test stems","http://snowball.tartarus.org/algorithms/english/stemmer.html")
{
    CHECK(stem("consign") == "consign");
    CHECK(stem("consigned") == "consign");
    CHECK(stem("consigning") == "consign");
    CHECK(stem("consignment") == "consign");
    CHECK(stem("consist") == "consist");
    CHECK(stem("consisted") == "consist");
    CHECK(stem("consistency") == "consist");
    CHECK(stem("consistent") == "consist");
    CHECK(stem("consistently") == "consist");
    CHECK(stem("consisting") == "consist");
    CHECK(stem("consists") == "consist");
    CHECK(stem("consolation") == "consol");
    CHECK(stem("consolations") == "consol");
    CHECK(stem("consolatory") == "consolatori");
    CHECK(stem("console") == "consol");
    CHECK(stem("consoled") == "consol");
    CHECK(stem("consoles") == "consol");
    CHECK(stem("consolidate") == "consolid");
    CHECK(stem("consolidated") == "consolid");
    CHECK(stem("consolidating") == "consolid");
    CHECK(stem("consoling") == "consol");
    CHECK(stem("consolingly") == "consol");
    CHECK(stem("consols") == "consol");
    CHECK(stem("consonant") == "conson");
    CHECK(stem("consort") == "consort");
    CHECK(stem("consorted") == "consort");
    CHECK(stem("consorting") == "consort");
    CHECK(stem("conspicuous") == "conspicu");
    CHECK(stem("conspicuously") == "conspicu");
    CHECK(stem("conspiracy") == "conspiraci");
    CHECK(stem("conspirator") == "conspir");
    CHECK(stem("conspirators") == "conspir");
    CHECK(stem("conspire") == "conspir");
    CHECK(stem("conspired") == "conspir");
    CHECK(stem("conspiring") == "conspir");
    CHECK(stem("constable") == "constabl");
    CHECK(stem("constables") == "constabl");
    CHECK(stem("constance") == "constanc");
    CHECK(stem("constancy") == "constanc");
    CHECK(stem("constant") == "constant");
    CHECK(stem("knack") == "knack");
    CHECK(stem("knackeries") == "knackeri");
    CHECK(stem("knacks") == "knack");
    CHECK(stem("knag") == "knag");
    CHECK(stem("knave") == "knave");
    CHECK(stem("knaves") == "knave");
    CHECK(stem("knavish") == "knavish");
    CHECK(stem("kneaded") == "knead");
    CHECK(stem("kneading") == "knead");
    CHECK(stem("knee") == "knee");
    CHECK(stem("kneel") == "kneel");
    CHECK(stem("kneeled") == "kneel");
    CHECK(stem("kneeling") == "kneel");
    CHECK(stem("kneels") == "kneel");
    CHECK(stem("knees") == "knee");
    CHECK(stem("knell") == "knell");
    CHECK(stem("knelt") == "knelt");
    CHECK(stem("knew") == "knew");
    CHECK(stem("knick") == "knick");
    CHECK(stem("knif") == "knif");
    CHECK(stem("knife") == "knife");
    CHECK(stem("knight") == "knight");
    CHECK(stem("knightly") == "knight");
    CHECK(stem("knights") == "knight");
    CHECK(stem("knit") == "knit");
    CHECK(stem("knits") == "knit");
    CHECK(stem("knitted") == "knit");
    CHECK(stem("knitting") == "knit");
    CHECK(stem("knives") == "knive");
    CHECK(stem("knob") == "knob");
    CHECK(stem("knobs") == "knob");
    CHECK(stem("knock") == "knock");
    CHECK(stem("knocked") == "knock");
    CHECK(stem("knocker") == "knocker");
    CHECK(stem("knockers") == "knocker");
    CHECK(stem("knocking") == "knock");
    CHECK(stem("knocks") == "knock");
    CHECK(stem("knopp") == "knopp");
    CHECK(stem("knot") == "knot");
    CHECK(stem("knots") == "knot");
}

}   // anonymous namespace


int main(int argc, char *const argv[])
{
#if defined(_MSC_VER)  &&  defined(_DEBUG)
    _CrtSetDbgFlag(_CrtSetDbgFlag(_CRTDBG_REPORT_FLAG) | _CRTDBG_LEAK_CHECK_DF);
//    _CrtSetBreakAlloc(6012);
#endif

    Catch::Session session;
    Catch::ConfigData &config_data = session.configData();
    config_data.showDurations = Catch::ShowDurations::OrNot::Always;

    int returnCode = session.applyCommandLine(argc, argv);
    if (returnCode != 0)
        return returnCode;

    return session.run();
}
