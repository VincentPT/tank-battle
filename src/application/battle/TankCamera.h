#pragma once
#include "Engine/DrawableObject.h"
#include "ObjectViewContainer.h"

typedef std::list<std::shared_ptr<SnapshotObject>> SnapshotRefObjects;

class TankCamera : public GameComponent {
protected:
	std::shared_ptr<ObjectViewContainer> _objectViewContainer;
	float _range;
	float _viewAngle;
	float _lastScanAt;
	std::vector<ci::vec2> _viewArea;
	SnapshotRefObjects _seenObjects;
public:
	TankCamera(const std::shared_ptr<ObjectViewContainer>& bjectViewContainer, float viewAngle = glm::pi<float>()/4);
	~TankCamera();
	void setRange(float range);
	float getRange() const;

	void update(float t);
	void draw();

	const std::shared_ptr<ObjectViewContainer>& getView() const;
	const SnapshotRefObjects& getSeenObjects() const;
};