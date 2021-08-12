#include <iostream>

#include <libOTe/TwoChooseOne/IknpOtExtSender.h>
#include <libOTe/TwoChooseOne/IknpOtExtReceiver.h>

#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/Timer.h>
#include <libOTe/Base/naor-pinkas.h>
#include <cryptoTools/Common/CLP.h>
#include <cryptoTools/Common/Log.h>

#include "SRSender.h"
#include "SRReceiver.h"

using namespace osuCrypto;
using namespace std;

block seed = ZeroBlock;

u64 sender_size;
u64 receiver_size;
u64 num_threads = 8;

string ip = "127.0.0.1:12345";

void sender() {
	IOService ios;
	Session session(ios, ip, EpMode::Server);
	vector<Channel> chls(num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();
	vector<block> senderSet(sender_size);

	for (auto i = 0; i < sender_size / 2; ++i) {
		senderSet[i] = toBlock(i + 1);
	}

	for (auto i = sender_size / 2; i < sender_size; ++i) {
		senderSet[i] = toBlock(i + receiver_size + 1 - sender_size / 2);
	}

	Timer timer;
	SRSender sender;
	sender.setTimer(timer);

	sender.setSenderSet(senderSet, receiver_size);
	timer.reset();
	timer.setTimePoint("before output");
	sender.output(chls);
	timer.setTimePoint("after output");

	cout << IoStream::lock;
	cout << "Sender:\n";
	/*for (auto& i : senderSet)
	{
		cout << i << endl;
	}*/
	cout << timer << endl;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << "recv: " << recv / 1024.0 / 1024.0 << "MB sent:" << sent / 1024.0 / 1024.0 << "MB" << endl;
	cout << IoStream::unlock;

}

void receiver() {
	Timer timer;
	IOService ios;
	Session session(ios, ip, EpMode::Client);
	vector<Channel> chls(num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();

	vector<block> receiverSet(receiver_size);
	for (auto i = 0; i < receiver_size; ++i) {
		receiverSet[i] = toBlock(i + 1);
	}

	SRReceiver receiver;
	receiver.setTimer(timer);
	receiver.setReceiverSet(receiverSet, sender_size);
	timer.reset();
	timer.setTimePoint("before output");
	auto res = receiver.output(chls);
	timer.setTimePoint("after output");

	cout << IoStream::lock;
	cout << "Receiver:\n";
	/*for (auto& i : res)
	{
		cout << i << endl;
	}*/
	cout << res.size() << endl;
	cout << endl;
	cout << timer;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << "recv: " << recv / 1024.0 / 1024.0 << "MB sent:" << sent / 1024.0 / 1024.0 << "MB" << endl;
	cout << IoStream::unlock;
}

int main(int argc, char** argv)
{
	srand(time(NULL));

	sender_size = 1ull << atoi(argv[1]);
	receiver_size = 1ull << atoi(argv[2]);
	num_threads = atoi(argv[3]);
	cout << "sender_size:" << sender_size << " vs receiver_size:" << receiver_size << ", num_threads:" << num_threads << endl;
	auto recver_thrd = std::thread(receiver);
	auto sender_thrd = std::thread(sender);
	recver_thrd.join();
	sender_thrd.join();

	return 0;
}
