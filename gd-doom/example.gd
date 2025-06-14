extends Node

var example: ExampleClass
var render_units: GDRenderUnitMesh

func _ready() -> void:
	example = ExampleClass.new()
	example.print_type(example)
	example.init()
	
	render_units = GDRenderUnitMesh.new()
	render_units.init()

func _process(delta: float) -> void:
	example.tick()
