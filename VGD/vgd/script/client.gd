extends Node
var vgd;

func _ready():
	vgd = load("res://bin/vgd.gdns").new()
	add_child(vgd)