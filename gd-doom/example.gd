extends Node

var example: ExampleClass

func _ready() -> void:
	example = ExampleClass.new()
	example.print_type(example)
	example.init()

func _process(delta: float) -> void:
	example.tick()
