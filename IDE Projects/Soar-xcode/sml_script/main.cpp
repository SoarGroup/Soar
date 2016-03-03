#include "sml_Client.h"
#include "agent.h"
#include "sml_EmbeddedConnectionSynch.h"
#include "sml_KernelSML.h"
#include "sml_AgentSML.h"

using namespace sml;
using namespace std;

int main(int argc, char* argv[])
{
	Kernel* kernel = sml::Kernel::CreateKernelInCurrentThread(true);
	Agent* agent = kernel->CreateAgent("Leak");

	agent->ExecuteCommandLine(
R"raw(
smem --set learning on
smem --set timers three

#wma --set forgetting on
#wma --set petrov-approx on
#wma --set activation on

watch 0

smem --add {(<s> ^test 5)}

sp {propose*init
	(state <s>	-^name
				^superstate nil)
-->
	(<s>	^operator.name init)
}

sp {apply*init
	(state <s>	^operator.name init)
-->
	(<s>	^name smem-leak-test
			^storage-container <sc>
			^id 1)
}

sp {propose*smem-query
	(state <s>	^name smem-leak-test
				-^smem.command.query)
-->
	(<s>	^operator.name smem-query)
}

sp {apply*smem-query
	(state <s> ^operator.name smem-query
				^smem.command <c>)
-->
	(<c>	^query.test 5)
}

sp {propose*store-result
	(state <s>	^smem.result.success <q>
				^id <id>
				-^storage-container.result.id <id>)
	(<q>	^test 5)
-->
	(<s>	^operator.name store-result)
}

sp {apply*store-result
	(state <s>	^operator.name store-result
				^smem.result.retrieved <q>
				^id <id>
				^storage-container <sc>
				^smem.command <command>)
	(<command>	^query <query>)
-->
	(<s>	^id <id> -)
	(<s>	^id (+ <id> 1))
	(<command>	^query <query> -)
	(<sc>	^result <container>)
	(<container>	^lti <q>
					^id <id>)
}

sp {elaboration*current-minus
	(state <s>	^id <id>)
-->
	(<s>	^deletion (- <id> 10))
}

sp {apply*store-result-removal
	(state <s>	^operator.name store-result
				^deletion <id>
				^storage-container <c>)
	(<c>	^result <r>)
	(<r>	^id <id>)
-->
	(<c>	^result <r> -)
}
)raw");

	smem_timer_container* container = dynamic_cast<EmbeddedConnectionSynch*>(kernel->GetConnection())->GetKernelSML()->GetAgentSML(agent->GetAgentName())->GetSoarAgent()->smem_timers;

	std::vector<soar_module::timer*> timers;

	timers.push_back(container->total);
	timers.push_back(container->storage);
	timers.push_back(container->ncb_retrieval);
	timers.push_back(container->query);
	timers.push_back(container->api);
	timers.push_back(container->init);
	timers.push_back(container->hash);
	timers.push_back(container->act);

	auto get_timers = [timers]() -> std::map<std::string, double>
	{
		std::map<std::string, double> result;

		for (soar_module::timer* timer : timers)
		{
			result[timer->get_name()] = timer->value();
		}

		return result;
	};

	static const char DELIMITER = ',';

	auto print_timer = [](std::map<std::string, double> timer, std::ostream* output)
	{
		for (auto pair : timer)
		{
			*output << DELIMITER << pair.second;
		}
	};

	auto print_timer_header = [](std::map<std::string, double> timer, std::ostream* output)
	{
		for (auto pair : timer)
		{
			*output << DELIMITER << pair.first;
		}
	};

	auto reset_timers = [timers]()
	{
		for (soar_module::timer* timer : timers)
		{
			timer->reset();
		}
	};

	std::ostream* output = &std::cout;

	ofstream out("out.csv");
	output = &out;

	std::map<std::string, double> query, store_result;

	agent->RunSelf(20, sml_DECISION);

	reset_timers();
	agent->RunSelf(1, sml_DECISION);
	query = get_timers();

	reset_timers();
	agent->RunSelf(1, sml_DECISION);
	store_result = get_timers();

	*output << "Query";
	print_timer_header(query, output);
	*output << DELIMITER << "Store";
	print_timer_header(store_result, output);
	*output << std::endl;

	print_timer(query, output);
	*output << DELIMITER;
	print_timer(store_result, output);
	*output << endl;

	for (int i = 0;;++i)
	{
		reset_timers();
		agent->RunSelf(1, sml_DECISION);

		auto timer = get_timers();

		if (i % 2 == 1)
			*output << DELIMITER;

		print_timer(timer, output);

		if (i % 2 == 1)
			*output << std::endl;
	}

	out.close();

	return 0;
}
