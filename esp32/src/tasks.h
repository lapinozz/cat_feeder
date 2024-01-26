struct Task {
  Commands cmd;

  inline static constexpr size_t MaxArgCount = 4;

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

struct TaskResult {
  uint8_t argCount = 0;
  int args[Task::MaxArgCount] = {};
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