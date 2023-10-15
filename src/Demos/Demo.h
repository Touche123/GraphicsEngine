#pragma once

#include "../SceneBase.h"

class Demo : public SceneBase {

public:
	Demo();
	~Demo() = default;

	MAKE_MOVE_ONLY(Demo);

	void Init(const std::string_view sceneName) override;
};