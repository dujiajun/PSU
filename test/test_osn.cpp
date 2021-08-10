//
// Created by dujiajun on 2021/8/9.
//

#include <iostream>
#include <vector>
#include <thread>

#include "cryptoTools/Network/IOService.h"

#include "OSNReceiver.h"
#include "OSNSender.h"

using namespace osuCrypto;
using namespace std;

vector<block> sender_shares;
vector<block> receiver_shares;
vector<block> receiver_set;
vector<int> permutation;
string ip = "127.0.0.1:12345";
size_t num_threads = 1;


void sender(size_t size)
{
	IOService ios;
	Session session(ios, ip, EpMode::Server);
	vector<Channel> chls;
	for (size_t i = 0; i < num_threads; i++)
	{
		chls.emplace_back(session.addChannel());
	}

	OSNSender osn;
	osn.init(size, 1);
	Timer timer;
	osn.setTimer(timer);
	timer.setTimePoint("before run_osn");
	sender_shares = osn.run_osn(chls);
	/*for (size_t i = 0; i < shares.size(); i++)
	{
		cout << i << " " << shares[i] << endl;
	}*/
	timer.setTimePoint("after run_osn");
	permutation = osn.dest;
	cout << IoStream::lock;
	cout << "Sender:" << endl;
	cout << timer << endl;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << sent / 1024.0 / 1024.0 << "MB sent and " << recv / 1024.0 / 1024.0
		<< "MB recv" << endl;
	cout << IoStream::unlock;
}

void receiver(size_t size)
{
	receiver_set.resize(size);
	for (size_t i = 0; i < receiver_set.size(); i++)
	{
		receiver_set[i] = toBlock(0, i);
	}
	IOService ios;
	Session session(ios, ip, EpMode::Client);
	vector<Channel> chls;
	for (size_t i = 0; i < num_threads; i++)
	{
		chls.emplace_back(session.addChannel());
	}

	OSNReceiver osn;
	osn.init(receiver_set.size(), 1);
	Timer timer;
	osn.setTimer(timer);
	timer.setTimePoint("before run_osn");
	receiver_shares = osn.run_osn(receiver_set, chls);
	timer.setTimePoint("after run_osn");
	/*
	for (size_t i = 0; i < shares.size(); i++)
	{
		cout << i << " " << shares[i] << endl;
	}*/
	cout << IoStream::lock;
	cout << "Receiver:" << endl;
	cout << timer << endl;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << sent / 1024.0 / 1024.0 << "MB sent and " << recv / 1024.0 / 1024.0
		<< "MB recv" << endl;
	cout << IoStream::unlock;
}

int check_result(size_t size)
{
	int correct_cnt = 0;
	for (auto i = 0; i < size; i++)
	{
		block tmp = sender_shares[i] ^ receiver_shares[i];
		if (tmp == receiver_set[permutation[i]])
		{
			correct_cnt++;
		}
	}
	return correct_cnt;
}
int main(int argc, char** argv)
{
	size_t size = 1 << atoi(argv[1]);
	num_threads = atoi(argv[2]);
	auto sender_thrd = thread(sender, size);
	auto receiver_thrd = thread(receiver, size);
	sender_thrd.join();
	receiver_thrd.join();
	if (size == check_result(size))
		cout << "Correct!" << endl;
	else
		cout << "Wrong!" << endl;
	return 0;
}
