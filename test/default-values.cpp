#include <cassert>
#include <cstdlib>

#include <rtosc/rtosc.h>
#include <rtosc/ports.h>
#include <rtosc/port-sugar.h>

#include "common.h"

using namespace rtosc;

static const Ports ports = {
    {"A::i", rDefault(64) rDoc("..."), NULL, NULL },
    {"B#2::i", rOpt(-2, Master) rOpt(-1, Off)
        rOptions(Part1, Part2) rDefault(Off), NULL, NULL },
    {"C::T", rDefault(false), NULL, NULL},
    {"D::f", rDefault(1.0), NULL, NULL},
    {"E::i", "", NULL, NULL}
};

void simple_default_values()
{
    assert_str_eq("64", get_default_value("A", ports, NULL),
        "get simple default value from int", __LINE__);
    assert_str_eq("Off", get_default_value("B0", ports, NULL),
        "get simple default value from options array (element 0)", __LINE__);
    assert_str_eq("Off", get_default_value("B1", ports, NULL),
        "get simple default value from options array (element 1)", __LINE__);
    assert_str_eq("false", get_default_value("C", ports, NULL),
        "get simple default value from toggle", __LINE__);
    assert_str_eq("1.0", get_default_value("D", ports, NULL),
        "get simple default value from float", __LINE__);
    assert_null(get_default_value("E", ports, NULL),
        "get default value where there is none", __LINE__);
}

struct Envelope
{
    static const rtosc::Ports& ports;
    std::size_t env_type;
};

// preset 0: volume low (30), attack slow (40)
// preset 1: all knobs high
static const Ports envelope_ports = {
    {"volume::i", rDefaultDepends(env_type) rMap(default 0, 30) rMap(default 1, 127), NULL, NULL },
    {"attack_rate::i", rDefaultDepends(env_type) rMap(default 0, 40) rMap(default 1, 127), NULL, NULL },
    {"env_type::i", rDefault(0), NULL, [](const char*, RtData& d) {
         d.reply("env_type", "i", (static_cast<Envelope*>(d.obj))->env_type); } }
};

const rtosc::Ports& Envelope::ports = envelope_ports;

void envelope_types()
{
    // by default, the envelope has preset 0, i.e. volume low (30), attack slow (40)
    assert_str_eq("30", get_default_value("volume", envelope_ports, NULL),
        "get dependent default value (without runtime) (part 1)", __LINE__);
    assert_str_eq("40", get_default_value("attack_rate", envelope_ports, NULL),
        "get dependent default value (without runtime) (part 2)", __LINE__);

    Envelope e1, e2, e3;

    e1.env_type = 0;
    e2.env_type = 1;
    e3.env_type = 2; // fake envelope type - will raise an error later

    assert_str_eq("30", get_default_value("volume", envelope_ports, &e1),
        "get dependent default value (with runtime) (part 1)", __LINE__);
    assert_str_eq("40", get_default_value("attack_rate", envelope_ports, &e1),
        "get dependent default value (with runtime) (part 2)", __LINE__);

    assert_str_eq("127", get_default_value("volume", envelope_ports, &e2),
        "get dependent default value (with runtime) (part 3)", __LINE__);
    assert_str_eq("127", get_default_value("attack_rate", envelope_ports, &e2 ),
        "get dependent default value (with runtime) (part 4)", __LINE__);

    bool error_caught = false;
    try {
        get_default_value("volume", envelope_ports, &e3);
    }
    catch(std::logic_error e) {
        error_caught = true;
        assert_str_eq("get_default_value(): "
                      "value depends on a variable which is "
                      "out of range of the port's default values ",
                      e.what(),
                      "matching errror message while getting "
                      "incorrect default value", __LINE__);
    }
    assert_true(error_caught,
                "get default value depending on a variable which is "
                "out of range of the port's default values", __LINE__);
}

void presets()
{
    // for presets, it would be exactly the same,
    // with only the env_type port being named as "preset" port.
}


int main()
{
    simple_default_values();
    envelope_types();
    presets();
    return test_summary();
}
