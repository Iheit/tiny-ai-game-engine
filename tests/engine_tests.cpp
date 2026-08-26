#include "engine.hpp"
#include "script.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace tiny;Scene s;Entity a;a.id=1;a.name="Player";a.kind=EntityKind::Cube;a.transform.position={1,2,3};a.material.color={.2f,.4f,.8f,1};a.dynamic=true;s.entities.push_back(a);s.cameraId=0;s.nextId=2;auto text=serialize(s);Scene r;std::string err;assert(deserialize(text,r,err));assert(r.entities.size()==1&&r.entities[0].name=="Player"&&r.entities[0].dynamic&&r.entities[0].transform.position.z==3);auto c=script::compile("speed = 5\nstart:\n    say \"Hello\"\n    move 0 0 1\nupdate:\n    rotate 0 1 0\n");assert(c.ok());bool said=false;script::VM vm;vm.setNative([&](const std::string&n,const std::vector<std::string>&){if(n=="say")said=true;});assert(vm.run(c.program));assert(said);auto bad=script::compile("start:\n    nonsense\n");assert(!bad.ok());std::cout<<"TinyEngineTests: scene + TinyScript OK\n";return 0;}
