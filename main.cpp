//#include <QCoreApplication>

#include "include/fcgiapp.h"
#include <QDebug>
#include <QByteArray>
#include <thread>
#include <atomic>

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
extern "C" {
//void __cxa_throw(          void* thrown_exception,
//			   class type_info *tinfo,
//			   void (*dest)(void*)){
//	std::exception* E = (std::exception*) thrown_exception;
//	//easier to access in the core dump
//	std::string miao = E->what();
//	printf("Ora crasho perché %s\n", miao.c_str());
//	stacker(1);
//	int*a = 0;
//	*a = 5;
//}


int select(int nfds, fd_set *readfds, fd_set *writefds,
	   fd_set *exceptfds, struct timeval *timeout){
	printf("Ora crasho perché hai usato SELECT! \n");
	int*a = 0;
	*a = 5;
}
}


std::atomic<int> rq{0};
int cry (int socketId ){
	FCGX_Request request;
	if (FCGX_InitRequest(&request, socketId, 0) != 0) {
		qDebug() << "FCGX_InitRequest boom!";
		return 0;
	}
	for(;;){
		rq++;
		if (FCGX_Accept_r(&request) == -1) {
			qDebug() << "can not accept";
			return 0;
		}

		auto http_protocol = FCGX_GetParam("SCHEME", request.envp);
		auto uri = FCGX_GetParam("QUERY_STRING", request.envp);
		auto remote_ip = FCGX_GetParam("REMOTE_ADDR", request.envp);

		QByteArray boom;
		boom.append("Status:200\r\n");
		boom.append("Content-Type:text/html; charset=utf-8\r\n\r\n");
		boom.append("SRLY!!! \n");
		boom.append(QByteArray::number(rq.load()));

		FCGX_PutStr(boom.constData(), boom.size(), request.out);
		FCGX_Finish_r(&request);
	}
}
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>

using namespace std::chrono_literals;
int main(int argc, char *argv[]) {
	int socketId = 0;

	pid_t pid = getpid();
	rlimit val;

	int res = prlimit(pid, RLIMIT_OFILE, NULL, &val);
	printf("Old limits: soft=%lu; hard=%lu\n",val.rlim_cur, val.rlim_max);

	val.rlim_cur = 3000;
	res = prlimit(pid, RLIMIT_OFILE, &val, NULL);
	if(res == -1){
		printf("Impossible to set soft to %lld;  change /etc/security/limits.h and give me moar descriptor", val.rlim_cur);
		return 1;
	}
	printf("New limits: soft=%lu; hard=%lu\n",val.rlim_cur, val.rlim_max);

	//open random file just to push the count and crash select

	for (int x=0; x < 1500; x++) {
		int id = open("/dev/null",O_RDONLY);
		if (id < 0) {
			qDebug() << "invalid socket after " << x;
			return 0;
		}
	}

	FCGX_Init();

	//the bug is triggered when using unix sockets always, and once in a while for all
	socketId = FCGX_OpenSocket("../sock", 1480);//we run with at max 512 thread for now

	//is this usefull ?
	//rc = setsockopt(socketId, SOL_SOCKET,  SO_REUSEADDR, (char *)&on, sizeof(on));
	//int getsockopt(int sockfd, int level, int optname, void *optval,socklen_t *optlen);



	//ABSOLUTELY NOT HERE!
//	auto flags = fcntl(socketId, F_GETFL, 0);
//	if (-1 == flags){
//		perror("noooo!");
//	   return -1;
//	}
//	int success = fcntl(socketId, F_SETFL, flags | O_NONBLOCK);

//	if (success < 0)
//	{
//	  perror("fcntl() failed");
//	  close(socketId);
//	  exit(-1);
//	}
	if (socketId < 0) {
		qDebug() << "invalid socket";
		return 0;
	}

	for (int i = 0; i < 4; ++i) {
		std::thread* t = new std::thread(&cry, socketId);
		t->detach();
	}

	std::this_thread::sleep_for(999s);

}
