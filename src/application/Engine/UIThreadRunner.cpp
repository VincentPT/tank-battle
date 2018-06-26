#include "UIThreadRunner.h"

UIThreadRunner::UIThreadRunner() {
}

UIThreadRunner::~UIThreadRunner() {
}

void UIThreadRunner::postTask(UpdateTask&& task) {
	_tasks.pushMessage(std::move(task));
}

void UIThreadRunner::executeTasks(float t) {
	UpdateTask task;
	while (_tasks.tryPopMessage(task))
	{
		task(t);
	}
}