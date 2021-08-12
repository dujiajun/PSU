#include <iostream>

#include <cryptoTools/Common/Defines.h>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>
#include <cryptoTools/Common/Timer.h>

#include "PSUSender.h"
#include "PSUReceiver.h"
#include "SSCSender.h"
#include "SSCReceiver.h"
#include "SSSender.h"
#include "SSReceiver.h"
#include "SRCSender.h"
#include "SRCReceiver.h"
#include "SRSender.h"
#include "SRReceiver.h"

#include "utils.h"
#include <cmdline.h>

using namespace osuCrypto;
using namespace std;

void run_sender(const Context& context)
{
	IOService ios;
	Session session(ios, context.host, context.port, EpMode::Server);
	vector<Channel> chls(context.num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();

	PRNG prng(toBlock(123));
	vector<block> senderSet(context.sender_size);
	for (auto i = 0; i < context.sender_size; ++i)
	{
		senderSet[i] = prng.get<block>();
	}

	Timer timer;
	PSUSender* sender = nullptr;
	if (context.psu_type == 0)
	{
		sender = new SRSender;
	}
	else if (context.psu_type == 1)
	{
		sender = new SSSender;
	}
	else if (context.psu_type == 2)
	{
		sender = new SRCSender;
	}
	else if (context.psu_type == 3)
	{
		sender = new SSCSender;
	}
	if (sender == nullptr)
		return;

	sender->setContext(context);
	sender->setTimer(timer);

	sender->setSenderSet(senderSet, context.receiver_size);

	timer.reset();
	timer.setTimePoint("before output");
	sender->output(chls);
	timer.setTimePoint("after output");

	cout << IoStream::lock;
	cout << "Sender: " << typeid(*sender).name() << endl;
	cout << timer << endl;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << "recv: " << recv / 1024.0 / 1024.0 << "MB sent:" << sent / 1024.0 / 1024.0 << "MB "
		<< "total: " << (recv + sent) / 1024.0 / 1024.0 << "MB\n" << endl;
	cout << IoStream::unlock;
	delete sender;
}

void run_receiver(const Context& context)
{
	Timer timer;
	IOService ios;
	Session session(ios, context.host, context.port, EpMode::Client);
	vector<Channel> chls(context.num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();

	vector<block> receiverSet(context.receiver_size);
	PRNG prng(toBlock(123));

	for (auto i = 0; i < context.receiver_size; ++i)
	{
		receiverSet[i] = prng.get<block>();
	}

	PSUReceiver* receiver = nullptr;
	if (context.psu_type == 0)
	{
		receiver = new SRReceiver;
	}
	else if (context.psu_type == 1)
	{
		receiver = new SSReceiver;
	}
	else if (context.psu_type == 2)
	{
		receiver = new SRCReceiver;
	}
	else if (context.psu_type == 3)
	{
		receiver = new SSCReceiver;
	}
	if (receiver == nullptr)
		return;

	receiver->setContext(context);
	receiver->setTimer(timer);

	receiver->setReceiverSet(receiverSet, context.sender_size);

	timer.reset();
	timer.setTimePoint("before output");
	auto res = receiver->output(chls);
	timer.setTimePoint("after output");

	cout << IoStream::lock;
	cout << "Receiver: " << typeid(*receiver).name() << endl;
	cout << "Union size: " << res.size() << endl;
	cout << timer << endl;
	size_t sent = 0, recv = 0;
	for (auto& chl : chls)
	{
		sent += chl.getTotalDataSent();
		recv += chl.getTotalDataRecv();
	}
	cout << "recv: " << recv / 1024.0 / 1024.0 << "MB sent:" << sent / 1024.0 / 1024.0 << "MB "
		<< "total: " << (recv + sent) / 1024.0 / 1024.0 << "MB\n" << endl;
	cout << IoStream::unlock;
	delete receiver;
}

Context parse_arguments(int argc, char** argv)
{
	cmdline::parser parser;

	parser.add<size_t>("role", 'u', "role(0: unit, 1: sender, 2: receiver)", false, 0);
	parser.add<size_t>("psu", 'p', "psu type (0: shuffle receiver, 1: shuffle sender, 2: shuffle receiver with cuckoo hash, 3: shuffle sender with cuckoo hash)", false, 2);
	parser.add<string>("host", '\0', "host name", false, "127.0.0.1");
	parser.add<size_t>("port", '\0', "port number", false, 12345);
	parser.add<size_t>("sender", 's', "sender set size (log2)", false, 8);
	parser.add<size_t>("receiver", 'r', "receiver set size (log2)", false, 8);
	parser.add<size_t>("threads", 't', "threads", false, 1);
	parser.add<size_t>("osn_ot", 'o', "osn ot type (IKNP: 1, SILENT: 0)", false, 1);
	parser.add<size_t>("hashes", 'h', "cuckoo hash num", false, 4);
	parser.add<double>("scaler", '\0', "cuckoo scaler", false, 1.09);

	parser.parse_check(argc, argv);

	Context context;
	context.role = parser.get<size_t>("role");
	context.psu_type = parser.get<size_t>("psu");
	context.host = parser.get<string>("host");
	context.port = parser.get<size_t>("port");
	context.sender_size = 1 << parser.get<size_t>("sender");
	context.receiver_size = 1 << parser.get<size_t>("receiver");
	context.num_threads = parser.get<size_t>("threads");
	context.osn_ot_type = parser.get<size_t>("osn_ot");
	context.cuckoo_hash_num = parser.get<size_t>("hashes");
	context.cuckoo_scaler = parser.get<double>("scaler");

	return context;
}

int main(int argc, char** argv)
{
	Context context = parse_arguments(argc, argv);

	cout << "===arguments==="
		<< "\nrole:" << context.role
		<< "\npsu type:" << context.psu_type
		<< "\nsender_size:" << context.sender_size
		<< "\nreceiver_size:" << context.receiver_size
		<< "\nnum_threads:" << context.num_threads
		<< "\nosn ot type:" << context.osn_ot_type
		<< "\ncuckoo hash num:" << context.cuckoo_hash_num
		<< "\ncuckoo scaler:" << context.cuckoo_scaler
		<< "\n===arguments===\n\n";

	if (context.role == 0)
	{
		auto recver_thrd = std::thread(run_receiver, context);
		auto sender_thrd = std::thread(run_sender, context);
		recver_thrd.join();
		sender_thrd.join();
	}
	else if (context.role == 1)
	{
		run_sender(context);
	}
	else if (context.role == 2)
	{
		run_receiver(context);
	}

	return 0;
}
