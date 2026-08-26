# TinyScript v0.1

TinyScript is the simple gameplay language for Tiny AI Game Engine. It is designed for readability and game scripting rather than general-purpose systems programming.

## Design rules

- No semicolons or braces.
- Indentation defines blocks.
- Variables do not require type declarations.
- Scripts are attached to game objects.
- The engine provides game-oriented commands.
- v0.1 uses bytecode executed by a TinyScript VM.

## Values

`number`, `text`, `true`, `false`, `nothing`, `object`, and `list`.

## Variables

```tiny
health = 100
speed = 5.0
name = "Player"
alive = true
```

## Operators

`+ - * / == != < > <= >= and or not`

## Conditions

```tiny
if health <= 0
    alive = false
else
    say "Alive"
```

## Loops

```tiny
repeat 10
    say "Hello"

while health > 0
    health = health - 1
```

## Functions

```tiny
function damage amount
    health = health - amount

damage 10
```

## Game events

```tiny
start:
    say "Spawned"

update:
    if key W
        move 0 0 5 * time

destroy:
    say "Destroyed"
```

## Engine commands planned for v0.1

`move`, `rotate`, `scale`, `spawn`, `destroy`, `find`, `play`, `music`, `say`, `load`, `restart`, `quit`, `key`, `pressed`, `released`, `mouse`.

## Compiler pipeline

TinyScript source -> lexer -> parser -> bytecode compiler -> TinyScript bytecode -> VM -> engine API.

The VM is intentionally small and deterministic. Native engine functionality is exposed through a narrow host API rather than arbitrary native code execution.
