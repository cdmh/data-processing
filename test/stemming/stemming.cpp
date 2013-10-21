// Copyright (c) 2013 Craig Henderson
// Part of the Data Processing Library
// https://github.com/cdmh/dataproc

#include"stdafx.h"
#include"porter_stemming.h"

#define CATCH_CONFIG_RUNNER
#include"catch.hpp"

namespace { // anonymous namespace

using cdmh::data_processing::porter_stemmer;

TEST_CASE("stemmer/misc","miscellaneous and common errors")
{
    CHECK(porter_stemmer("possibly") == porter_stemmer("possible"));
    CHECK(porter_stemmer("archaeology") == porter_stemmer("archaeological"));

    // test common errors
    CHECK(porter_stemmer("argument") == "argum");
}

TEST_CASE("stemmer/test stems","http://snowball.tartarus.org/algorithms/english/stemmer.html")
{
    CHECK(porter_stemmer("consign") == "consign");
    CHECK(porter_stemmer("consigned") == "consign");
    CHECK(porter_stemmer("consigning") == "consign");
    CHECK(porter_stemmer("consignment") == "consign");
    CHECK(porter_stemmer("consist") == "consist");
    CHECK(porter_stemmer("consisted") == "consist");
    CHECK(porter_stemmer("consistency") == "consist");
    CHECK(porter_stemmer("consistent") == "consist");
    CHECK(porter_stemmer("consistently") == "consist");
    CHECK(porter_stemmer("consisting") == "consist");
    CHECK(porter_stemmer("consists") == "consist");
    CHECK(porter_stemmer("consolation") == "consol");
    CHECK(porter_stemmer("consolations") == "consol");
    CHECK(porter_stemmer("consolatory") == "consolatori");
    CHECK(porter_stemmer("console") == "consol");
    CHECK(porter_stemmer("consoled") == "consol");
    CHECK(porter_stemmer("consoles") == "consol");
    CHECK(porter_stemmer("consolidate") == "consolid");
    CHECK(porter_stemmer("consolidated") == "consolid");
    CHECK(porter_stemmer("consolidating") == "consolid");
    CHECK(porter_stemmer("consoling") == "consol");
    CHECK(porter_stemmer("consolingly") == "consol");
    CHECK(porter_stemmer("consols") == "consol");
    CHECK(porter_stemmer("consonant") == "conson");
    CHECK(porter_stemmer("consort") == "consort");
    CHECK(porter_stemmer("consorted") == "consort");
    CHECK(porter_stemmer("consorting") == "consort");
    CHECK(porter_stemmer("conspicuous") == "conspicu");
    CHECK(porter_stemmer("conspicuously") == "conspicu");
    CHECK(porter_stemmer("conspiracy") == "conspiraci");
    CHECK(porter_stemmer("conspirator") == "conspir");
    CHECK(porter_stemmer("conspirators") == "conspir");
    CHECK(porter_stemmer("conspire") == "conspir");
    CHECK(porter_stemmer("conspired") == "conspir");
    CHECK(porter_stemmer("conspiring") == "conspir");
    CHECK(porter_stemmer("constable") == "constabl");
    CHECK(porter_stemmer("constables") == "constabl");
    CHECK(porter_stemmer("constance") == "constanc");
    CHECK(porter_stemmer("constancy") == "constanc");
    CHECK(porter_stemmer("constant") == "constant");
    CHECK(porter_stemmer("knack") == "knack");
    CHECK(porter_stemmer("knackeries") == "knackeri");
    CHECK(porter_stemmer("knacks") == "knack");
    CHECK(porter_stemmer("knag") == "knag");
    CHECK(porter_stemmer("knave") == "knave");
    CHECK(porter_stemmer("knaves") == "knave");
    CHECK(porter_stemmer("knavish") == "knavish");
    CHECK(porter_stemmer("kneaded") == "knead");
    CHECK(porter_stemmer("kneading") == "knead");
    CHECK(porter_stemmer("knee") == "knee");
    CHECK(porter_stemmer("kneel") == "kneel");
    CHECK(porter_stemmer("kneeled") == "kneel");
    CHECK(porter_stemmer("kneeling") == "kneel");
    CHECK(porter_stemmer("kneels") == "kneel");
    CHECK(porter_stemmer("knees") == "knee");
    CHECK(porter_stemmer("knell") == "knell");
    CHECK(porter_stemmer("knelt") == "knelt");
    CHECK(porter_stemmer("knew") == "knew");
    CHECK(porter_stemmer("knick") == "knick");
    CHECK(porter_stemmer("knif") == "knif");
    CHECK(porter_stemmer("knife") == "knife");
    CHECK(porter_stemmer("knight") == "knight");
    CHECK(porter_stemmer("knightly") == "knight");
    CHECK(porter_stemmer("knights") == "knight");
    CHECK(porter_stemmer("knit") == "knit");
    CHECK(porter_stemmer("knits") == "knit");
    CHECK(porter_stemmer("knitted") == "knit");
    CHECK(porter_stemmer("knitting") == "knit");
    CHECK(porter_stemmer("knives") == "knive");
    CHECK(porter_stemmer("knob") == "knob");
    CHECK(porter_stemmer("knobs") == "knob");
    CHECK(porter_stemmer("knock") == "knock");
    CHECK(porter_stemmer("knocked") == "knock");
    CHECK(porter_stemmer("knocker") == "knocker");
    CHECK(porter_stemmer("knockers") == "knocker");
    CHECK(porter_stemmer("knocking") == "knock");
    CHECK(porter_stemmer("knocks") == "knock");
    CHECK(porter_stemmer("knopp") == "knopp");
    CHECK(porter_stemmer("knot") == "knot");
    CHECK(porter_stemmer("knots") == "knot");
}

TEST_CASE("stemmer/Step1a","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("caresses") == "caress");
    CHECK(porter_stemmer("ponies") == "poni");
    CHECK(porter_stemmer("ties") == "ti");
    CHECK(porter_stemmer("caress") == "caress");
    CHECK(porter_stemmer("cats") == "cat");
}

TEST_CASE("stemmer/Step1b","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("feed") == "feed");
    CHECK(porter_stemmer("agreed") == "agree");
    CHECK(porter_stemmer("plastered") == "plaster");
    CHECK(porter_stemmer("bled") == "bled");
    CHECK(porter_stemmer("motoring") == "motor");
    CHECK(porter_stemmer("sing") == "sing");
    CHECK(porter_stemmer("conflated") == "conflate");
    CHECK(porter_stemmer("troubled") == "trouble");
    CHECK(porter_stemmer("sized") == "size");
    CHECK(porter_stemmer("hopping") == "hop");
    CHECK(porter_stemmer("tanned") == "tan");
    CHECK(porter_stemmer("falling") == "fall");
    CHECK(porter_stemmer("hissing") == "hiss");
    CHECK(porter_stemmer("fizzed") == "fizz");
    CHECK(porter_stemmer("failing") == "fail");
    CHECK(porter_stemmer("filing") == "file");
}

TEST_CASE("stemmer/step1c","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("happy") == "happi");
    CHECK(porter_stemmer("sky") == "sky");
}

TEST_CASE("stemmer/step2","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("relational") == "relate");
    CHECK(porter_stemmer("conditional") == "condition");
    CHECK(porter_stemmer("rational") == "rational");
    CHECK(porter_stemmer("valenci") == "valence");
    CHECK(porter_stemmer("hesitanci") == "hesitance");
    CHECK(porter_stemmer("digitizer") == "digitize");
    CHECK(porter_stemmer("conformabli") == "conformable");
    CHECK(porter_stemmer("radicalli") == "radical");
    CHECK(porter_stemmer("differentli") == "different");
    CHECK(porter_stemmer("vileli") == "vile");
    CHECK(porter_stemmer("analogousli") == "analogous");
    CHECK(porter_stemmer("vietnamization") == "vietnamize");
    CHECK(porter_stemmer("predication") == "predicate");
    CHECK(porter_stemmer("operator") == "operate");
    CHECK(porter_stemmer("feudalism") == "feudal");
    CHECK(porter_stemmer("decisiveness") == "decisive");
    CHECK(porter_stemmer("hopefulness") == "hopeful");
    CHECK(porter_stemmer("callousness") == "callous");
    CHECK(porter_stemmer("formaliti") == "formal");
    CHECK(porter_stemmer("sensitiviti") == "sensitive");
    CHECK(porter_stemmer("sensibiliti") == "sensible");
}

TEST_CASE("stemmer/step3","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("triplicate") == "triplic");
    CHECK(porter_stemmer("formative") == "form");
    CHECK(porter_stemmer("formalize") == "formal");
    CHECK(porter_stemmer("electriciti") == "electric");
    CHECK(porter_stemmer("electrical") == "electric");
    CHECK(porter_stemmer("hopeful") == "hope");
    CHECK(porter_stemmer("goodness") == "good");
}

TEST_CASE("stemmer/step4","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("revival") == "reviv");
    CHECK(porter_stemmer("allowance") == "allow");
    CHECK(porter_stemmer("inference") == "infer");
    CHECK(porter_stemmer("airliner") == "airlin");
    CHECK(porter_stemmer("gyroscopic") == "gyroscop");
    CHECK(porter_stemmer("adjustable") == "adjust");
    CHECK(porter_stemmer("defensible") == "defens");
    CHECK(porter_stemmer("irritant") == "irrit");
    CHECK(porter_stemmer("replacement") == "replac");
    CHECK(porter_stemmer("adjustment") == "adjust");
    CHECK(porter_stemmer("dependent") == "depend");
    CHECK(porter_stemmer("adoption") == "adopt");
    CHECK(porter_stemmer("homologou") == "homolog");
    CHECK(porter_stemmer("communism") == "commun");
    CHECK(porter_stemmer("activate") == "activ");
    CHECK(porter_stemmer("angulariti") == "angular");
    CHECK(porter_stemmer("homologous") == "homolog");
    CHECK(porter_stemmer("effective") == "effect");
    CHECK(porter_stemmer("bowdlerize") == "bowdler");
}

TEST_CASE("stemmer/step5a","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("probate") == "probat");
    CHECK(porter_stemmer("rate") == "rate");
    CHECK(porter_stemmer("cease") == "ceas");
}

TEST_CASE("stemmer/step5b","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("controll") == "control");
    CHECK(porter_stemmer("roll") == "roll");
}

TEST_CASE("stemmer/stem too short","http://tartarus.org/martin/PorterStemmer/def.txt")
{
    CHECK(porter_stemmer("RELATE") == "RELATE");
    CHECK(porter_stemmer("PROBATE") == "PROBATE");
    CHECK(porter_stemmer("CONFLATE") == "CONFLATE");
    CHECK(porter_stemmer("PIRATE") == "PIRATE");
    CHECK(porter_stemmer("PRELATE") == "PRELATE");

    CHECK(porter_stemmer("DERIVATE") == "DERIV");
    CHECK(porter_stemmer("ACTIVATE") == "ACTIV");
    CHECK(porter_stemmer("DEMONSTRATE") == "DEMONSTR");
    CHECK(porter_stemmer("NECESSITATE") == "NECESSIT");
    CHECK(porter_stemmer("RENOVATE") == "RENOV");

    CHECK(porter_stemmer("DERIVATE") == porter_stemmer("DERIVE"));
    CHECK(porter_stemmer("ACTIVATE") == porter_stemmer("ACTIVE"));
    CHECK(porter_stemmer("DEMONSTRATE") == porter_stemmer("DEMONSTRABLE"));
    CHECK(porter_stemmer("NECESSITATE") == porter_stemmer("NECESSITOUS"));
}

}   // anonymous namespace


int main(int argc, char * const argv[])
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
