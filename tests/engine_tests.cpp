#include "engine.hpp"
#include <cassert>
#include <iostream>
int main(){using namespace tiny;Scene s;Entity a;a.id=1;a.name="Player";a.kind=EntityKind::Cube;a.transform.position={1,2,3};a.material.color={.2f,.4f,.8f,1};a.dynamic=true;s.entities.push_back(a);s.cameraId=0;s.nextId=2;auto text=serialize(s);Scene r;std::string err;assert(deserialize(text,r,err));assert(r.entities.size()==1);assert(r.entities[0].name=="Player");assert(r.entities[0].dynamic);assert(r.entities[0].transform.position.z==3);std::cout<<"TinyEngineTests: scene serialization OK\n";return 0;}
