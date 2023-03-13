// 2023-02-21 lot sizing sub problem


#include "GMLS.h"
using namespace std;

void SolveSubProblem(All_Values& Values, All_Lists& Lists)
{
	int prids_num = Values.prids_num;
	int machs_num = Values.machs_num;

	IloEnv Env_SP;
	IloModel Model_SP(Env_SP);
	IloCplex Cplex_SP(Env_SP);

	// vars
	IloArray<IloNumVar> X_vars(Env_SP);
	IloArray<IloNumVar> Y_vars(Env_SP);
	IloArray<IloNumVar> I_vars(Env_SP);
	IloArray<IloNumVar> Z_vars(Env_SP);

	for (int t = 0; t < prids_num; t++)
	{
		string X_name = "X_" + to_string(t + 1);
		string Y_name = "Y_" + to_string(t + 1);
		string I_name = "I_" + to_string(t + 1);

		X_vars.add(IloNumVar(Env_SP, 0, IloInfinity, ILOINT, X_name.c_str()));
		Y_vars.add(IloNumVar(Env_SP, 0, 1, ILOINT, Y_name.c_str()));
		I_vars.add(IloNumVar(Env_SP, 0, IloInfinity, ILOINT, I_name.c_str()));
	}

	for (int m = 0; m < machs_num; m++)
	{
		string Z_name = "Z_" + to_string(m + 1);
		Z_vars.add(IloNumVar(Env_SP, 0, 1, ILOINT, Z_name.c_str()));
	}

	// obj
	IloExpr obj_sum(Env_SP);
	for (int t = 0; t < prids_num; t++)
	{
		obj_sum -= Lists.primal_parameters[t].c_X * X_vars[t];
	}

	for (int t = 0; t < prids_num; t++)
	{
		obj_sum -= Lists.primal_parameters[t].c_Y * Y_vars[t];
	}

	for (int t = 0; t < prids_num; t++)
	{
		obj_sum -= Lists.primal_parameters[t].c_I * I_vars[t];
	}

	for (int m = 0; m < machs_num; m++)
	{
		obj_sum += Lists.master_solns_list[m] * Z_vars[m]; // sum W_m * Z_m
	}

	IloObjective Obj_OR = IloMaximize(Env_SP, obj_sum); // obj max
	Model_SP.add(Obj_OR);
	obj_sum.end();

	// number of con 1 == T
	for (int t = 0; t < prids_num; t++)
	{
		IloExpr con_sum(Env_SP);

		for (int m = 0; m < machs_num; m++)
		{
			con_sum += Lists.demand_matrix[m][t] * Z_vars[m]; // sum d_mt *  Z_m
		}

		if (t == 0)
		{
			Model_SP.add(X_vars[t] == con_sum + I_vars[t]);
		}

		if (t > 0)
		{
			Model_SP.add(X_vars[t] + I_vars[t - 1] == con_sum + I_vars[t]);
		}

		con_sum.end();
	}

	// number of con 2 == T 
	for (int t = 0; t < prids_num; t++)
	{
		Model_SP.add(X_vars[t] <= Values.machine_capacity * Y_vars[t]);
	}

	// uppdate machine capacity
	Values.machine_capacity = 0;

	printf("\n/////////// CPLEX SOLVING START ////////////\n\n");
	Cplex_SP.extract(Model_SP);
	Cplex_SP.exportModel("SubProblem1.lp");
	bool SP_flag = Cplex_SP.solve();

	//if (coalition_flag == 1)
	//{
	//	Cplex_SP.exportModel("LSMM1.lp");
	//}
	//if (coalition_flag == 2)
	//{
	//	Cplex_SP.exportModel("LSMM2.lp");
	//}
	//if (coalition_flag == 3)
	//{
	//	Cplex_SP.exportModel("LSMM3.lp");
	//}
	//if (coalition_flag == 4)
	//{
	//	Cplex_SP.exportModel("LSMM12.lp");
	//}
	//if (coalition_flag == 5)
	//{
	//	Cplex_SP.exportModel("LSMM13.lp");
	//}
	//if (coalition_flag == 6)
	//{
	//	Cplex_SP.exportModel("LSMM23.lp");
	//}
	//if (coalition_flag == 7)
	//{
	//	Cplex_SP.exportModel("LSMM123.lp");
	//}

	printf("\n/////////// CPLEX SOLVING END ////////////\n");

	int obj_val = Cplex_SP.getObjValue();
	printf("\n	Obj = %d\n", obj_val);
	cout << endl;

	for (int m = 0; m < machs_num; m++)
	{
		int soln_val = Cplex_SP.getValue(Z_vars[m]);
		printf("	Z_%d= %d\n", m + 1, soln_val);
		Lists.coalition_solns_list.push_back(soln_val);
	}
	cout << endl;

	for (int t = 0; t < prids_num; t++)
	{
		int soln_val = Cplex_SP.getValue(X_vars[t]);
		printf("	X_%d = %d\n", t + 1, soln_val);
	}
	cout << endl;

	for (int t = 0; t < prids_num; t++)
	{
		int soln_val = Cplex_SP.getValue(Y_vars[t]);
		printf("	Y_%d= %d\n", t + 1, soln_val);
	}
	cout << endl;

	for (int t = 0; t < prids_num; t++)
	{
		int soln_val = Cplex_SP.getValue(I_vars[t]);
		printf("	I_%d= %d\n", t + 1, soln_val);
	}	
	cout << endl;

	Env_SP.end();

	cout << endl;
}