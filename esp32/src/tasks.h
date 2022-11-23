struct Task {
  Commands cmd;

  static constexpr uint8_t MaxArgCount = 3;

  uint8_t argCount = 0;
  int args[MaxArgCount] = {};

  Task()
  {

  }

  template<typename...Args>
  Task(Commands c, Args...args) : cmd(c), argCount(sizeof...(args)), args{args...}
  {
  }
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