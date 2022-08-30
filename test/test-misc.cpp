#include <iostream>
#include <cstdio>
#include <chrono>
#include <atomic>
#include <cstdint>
#include <mutex>
#include <thread>
#include <vector>
#include <memory>
#include <unistd.h>
#include <pthread.h>
#include <filesystem>
#include <sys/file.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <semaphore.h>

struct MyTLSDeleter {
    MyTLSDeleter(int _threadIdx) : threadIdx(_threadIdx) {}
    void operator()(uint32_t* p) {
        delete p;
        printf("TLS var of thread %u was deleted\n", gettid());
    }

    int threadIdx;
};

thread_local std::shared_ptr<uint32_t> g;
std::recursive_mutex g_lock;
sem_t g_sema;
const int G_THREAD_COUNT = 4;
const int G_SEMAPHORE_INIT_VALUE = G_THREAD_COUNT / 2;

void thread_msg(int idx, const std::string& msg) {
    sem_wait(&g_sema);
    g_lock.lock();
    g_lock.lock();
    if (!g)
        g.reset(new uint32_t(0), MyTLSDeleter(idx));
    std::cout << "thread " << idx << " says: " << msg << std::endl;
    (*g)++;
    std::cout << "TLS var g of thread " << idx << " is: " << *g << std::endl;
    g_lock.unlock();
    g_lock.unlock();
    if (idx <= G_SEMAPHORE_INIT_VALUE)
        sem_post(&g_sema);
}

static_assert(sizeof(sockaddr_un) <= sizeof(sockaddr_storage));

int main() {
    printf("Current timestamp: %llu\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    printf("Current timestamp after sleep: %llu\n", std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
    printf("PID: %u\n", getpid());
    printf("CPU count: %u\n", std::thread::hardware_concurrency());
    printf("Total memory size: %ldKB\n", sysconf(_SC_PHYS_PAGES) * sysconf(_SC_PAGE_SIZE) / 1024);
    printf("Temporary dir: %s\n", getenv("TMPDIR"));
    FILE* f = fopen("/etc/resolv.conf", "r");
    if (f) {
        int tempfd = fileno(f);
        if (flock(tempfd, LOCK_SH) != 0)
            perror("Failed to lock /etc/resolv.conf");
        char buf[10000] = {0};
        fread(buf, 1, sizeof(buf), f);
        flock(tempfd, LOCK_UN);
        printf("Content of /etc/resolv.conf:\n%s\n", buf);
        fclose(f);
        f = nullptr;
    } else {
        printf("Failed to open /etc/resolv.conf\n");
    }
    f = fopen("/dev/urandom", "rb");
    if (f) {
        uint64_t random = 0;
        fread(&random, sizeof(random), 1, f);
        printf("Random from /dev/urandom: %llu\n", random);
        fclose(f);
        f = nullptr;
    } else {
        printf("Failed to open /dev/urandom\n");
    }

    std::filesystem::space_info si = std::filesystem::space("/etc");
    printf("Space info of /etc: total: %lluMB, free: %lluMB\n", si.capacity / 1024 / 1024, si.free / 1024 / 1024);
    if (sem_init(&g_sema, 0, G_SEMAPHORE_INIT_VALUE + 1) != 0) {
        perror("sem_init()");
        return 1;
    }
    std::vector<std::shared_ptr<std::thread>> threads;
    for (int i = 1; i <= G_THREAD_COUNT; i++)
        threads.push_back(std::make_shared<std::thread>(thread_msg, i, "Hello"));
    for (auto& t : threads)
        t->join();
    threads.clear();
    int sem_value;
    sem_getvalue(&g_sema, &sem_value);
    printf("Current semaphore value: %d\n", sem_value);
    sem_destroy(&g_sema);

    std::atomic<uint64_t> n(0);
    n++;
    std::cout << "Hello world" << std::endl << n.load() << std::endl;
    return 0;
}
