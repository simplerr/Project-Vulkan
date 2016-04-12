#pragma once

#include <string>

class StaticModel;

// TODO: This will later work like a factory, where the same model only gets loaded once
class ModelLoader
{
public:
	StaticModel* LoadModel(std::string filename);
private:

};
