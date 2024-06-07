#pragma once

#include "IObject.h"


class MeshObject :public IObject
{
public:
	MeshObject();
	virtual ~MeshObject();

public:
	virtual void Update() override;
	virtual void Render() override;

private:


};

