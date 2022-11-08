struct Task {
  Commands cmd;
  int args[1];
};

SemaphoreHandle_t tasksSemaphore = NULL;
CircularBuffer<Task, 32> tasks;

bool popTask(Task& task)
{
	xSemaphoreTake(tasksSemaphore, portMAX_DELAY);
  if(tasks.size() == 0)
  {
    xSemaphoreGive(tasksSemaphore);
    return false;
  }
  else
  {
    task = tasks.shift();
    xSemaphoreGive(tasksSemaphore);
    return true;
  }
}

void pushTask(const Task& task)
{
	xSemaphoreTake(tasksSemaphore, portMAX_DELAY);
  tasks.push(task);
	xSemaphoreGive(tasksSemaphore);
}

using TaskExec = void (*)(Task& task);