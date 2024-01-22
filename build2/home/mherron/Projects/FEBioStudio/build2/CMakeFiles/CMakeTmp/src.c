#include <pthread.h>

static void* test_func(void* data)
{
  return data;
}

int main(void)
{
  pthread_t thread;
  pthread_create(&thread, NULL, test_func, NULL);
  pthread_detach(thread);
  pthread_cancel(thread);
  pthread_join(thread, NULL);
  pthread_atfork(NULL, NULL, NULL);
  pthread_exit(NULL);

  return 0;
}

