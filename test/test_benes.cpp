#include <iostream>
#include <cryptoTools/Network/Session.h>
#include <cryptoTools/Network/Channel.h>
#include <cryptoTools/Network/IOService.h>

#include <vector>
#include <algorithm>
#include <cassert>
#include <bitset>

#include "BenesPSSender.h"
#include "BenesPSReceiver.h"
#include "Permutation.h"
#include "PermutationLayer.h"
#include "BenesNetwork.h"

using namespace osuCrypto;

using namespace std;

size_t set_size = 1ull << 10;
size_t log2_set_size = log2ceil(set_size);
size_t log2_permutation_size = log2_set_size >= 12 ? (log2_set_size / 2 + 1) : log2_set_size;
u64 num_threads = 8;

block seed = ZeroBlock;
vector<block> recv_share(set_size);      //receiver拿到的share
vector<block> sender_share(set_size);    //sender拿到的share
vector<size_t> pi(set_size);             //Permutation

string ip = "127.0.0.1:1212";
void benes_sender()
{
	//cout << "**************Sender****************" << endl;
	Timer timer;

	IOService ios;

	Session session(ios, ip, SessionMode::Client);
	vector<Channel> chls(num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();

	BenesPSSender s(set_size, log2_permutation_size, seed);
	s.setTimer(timer);
	srand(time(NULL));
	BenesNetwork network(set_size, log2_permutation_size);
	network.randomShuffle();

	pi = network.output();
	size_t st_count = 0;
	for (size_t i = 0; i < network.layer_counts; i++) {
		st_count = st_count + network.layers[i].permutation_counts;
	}
	timer.reset();
	timer.setTimePoint("before output");
	sender_share = s.output(network, st_count, network.layer_counts - 1, size_t(2), chls);
	timer.setTimePoint("after output");
	cout << IoStream::lock;
	cout << "Sender:" << endl;
	cout << timer << endl;
	cout << IoStream::unlock;
}

void benes_recver()
{
	//cout << "**************Receiver****************" << endl;
	Timer timer;

	IOService ios;
	Session session(ios, ip, SessionMode::Server);
	vector<Channel> chls(num_threads);
	for (auto& chl : chls)
		chl = session.addChannel();

	BenesPSReceiver r(set_size, log2_permutation_size, seed);  //持有集合的一方
	r.setTimer(timer);
	vector<block> x(set_size);
	for (size_t i = 0; i < set_size; i++)
	{
		x[i] = toBlock(0, i);
	}

	BenesNetwork network(set_size, log2_permutation_size);
	size_t st_count = 0;
	for (size_t i = 0; i < network.layer_counts; i++) {
		st_count = st_count + network.layers[i].permutation_counts;
		//cout << "layer " << i << "-th's st count: " << network.layers[i].permutation_counts << endl;
	}
	timer.reset();
	timer.setTimePoint("before output");
	recv_share = r.output(network, x, st_count, network.layer_counts - 1, 2, chls);
	timer.setTimePoint("after output");
	cout << IoStream::lock;
	cout << "Receiver:" << endl;
	cout << timer << endl;
	cout << IoStream::unlock;
}


int main(int argc, char* argv[])
{
	set_size = 1ull << atoi(argv[1]);
	log2_set_size = atoi(argv[1]);
	log2_permutation_size = atoi(argv[2]);
	num_threads = atoi(argv[3]);

	cout << log2_set_size << " " << log2_permutation_size << " " << num_threads << endl;
	auto recver_thrd = std::thread(benes_recver);
	auto sender_thrd = std::thread(benes_sender);
	sender_thrd.join();
	recver_thrd.join();
	vector<block> x(set_size);
	size_t flag = 0;
	for (size_t i = 0; i < set_size; i++)
	{
		x[i] = toBlock(0, i);
	}
	for (int i = 0; i < set_size; i++) {
		//cout << "recover set: " << _mm_xor_si128(recv_share[i], sender_share[i])<<"------" << pi[i] <<endl;
		block recover = _mm_xor_si128(recv_share[i], sender_share[i]);
		if (recover != x[pi[i]]) {
			flag = flag + 1;
		}
	}
	if (flag == 0) { cout << "=======CORRECT!!!!!========" << endl; }
	else cout << flag << endl;
	return 0;
}