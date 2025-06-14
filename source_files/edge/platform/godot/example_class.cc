#include "example_class.h"
#include <con_main.h>

void GDD_Init(int argc, char *argv[]);
void GDD_Tick();

void ExampleClass::_bind_methods()
{
    godot::ClassDB::bind_method(D_METHOD("print_type", "variant"), &ExampleClass::print_type);
    godot::ClassDB::bind_method(D_METHOD("init"), &ExampleClass::init);
    godot::ClassDB::bind_method(D_METHOD("tick"), &ExampleClass::tick);
}

void ExampleClass::print_type(const Variant &p_variant) const
{
    print_line(vformat("GD-Doom Type: %d", p_variant.get_type()));
}

void ExampleClass::init() const
{
	GDD_Init(0, nullptr);

    TryConsoleCommand("map map01");
}

void ExampleClass::tick() const
{
    GDD_Tick();
}
