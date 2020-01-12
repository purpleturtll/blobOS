#pragma once

#include <string>
#include <vector>
#include <array>
#include <iostream>
#include "Memory.h"
#include "process.h"
//#include "procesor.h"

extern Memory memory;

ProcTree PTree;

using namespace std;

shared_ptr<PCB> PCB_return; // zmienna pomocnicza dla find_pid
bool find_bool = 0; // zmienna pomocnicza dla find_pid 
shared_ptr<PCB> PCB_name_return; // zmienna pomocnicza dla find_name
bool find_bool_name = 0; // zmienna pomocnicza dla find_name 

void PCB::show_info() {

	cout << "NAME: " << this->name << endl;
	cout << "PID: "<< this->pid<< endl;
	cout << "Parent: " << this->parent_pid << endl;
	cout << "State: "; 
	switch (this->state) {
	case 0:
		cout << "ready" << endl;
		break;
	case 1:
		cout << "run" << endl;
		break;
	case 2:
		cout << "wait" << endl;
		break;
	case 3:
		cout << "terminated" << endl;
		break;
	}
	//cout << "bool kill" << this->bool_kill << endl;

}

void PCB::copy_register(array<int, 5> &cpu_register) //kopiowanie zawartości rejestru do PCB
{ 
    for (int i = 0; i < 5; i++)
    {
        this->register_copy[i] = cpu_register[i];
    }
}

void PCB::restore_register(array<int, 5> &cpu_register) // przywracanie zawartości rejestru
{ 
    for (int i = 0; i < 5; i++)
    {
        cpu_register[i] = this->register_copy[i];
    }
}

/*
void PCB::kill() // zmiana stanu procesu za zakończony
{
    process_state state = terminated;
    this->change_state(state);
}*/

void PCB::change_state(process_state &new_state) // zmiana stanu procesu (planista)
{
    this->state = new_state;
	// planista - oczekiwanie
	// Planist::manage();
}

bool PCB::null_vector_child() { // zwraca 1 gdy pusty i rodzic może się zakończyć 

	if (this->children_vector.size() == 0) {
		return 1;
	}
	else {
		return 0;
	}
};

shared_ptr<PCB> PCB::get_kid_pid(int &find_pid)
{
    if(this->children_vector.size() == 0){
        return nullptr;
    }

    for(int i=0; i<this->children_vector.size(); i++){ // przeszukujemy wektor dzieci w celu znalezienia szukanego PID
        if(this->children_vector[i]->pid == find_pid){
            return children_vector[i];  // zwracamy wskaźnik na PCB o znalezionym PID
        }
    }

    for(int i=0; i<this->children_vector.size(); i++){ // jeśli nie znajdziemy szukanego PID dla każdego z dzieci wywołujemy funkcję get_kid 
        children_vector[i]->get_kid_pid(find_pid); // rekurencja
    }

    return nullptr; // jeśli nie znajdziemy PID , zwracamy nullptr
}

shared_ptr<PCB> PCB::get_kid_name(string &find_name)
{
	if (this->children_vector.size() == 0) {
		return nullptr;
	}

	for (int i = 0; i < this->children_vector.size(); i++) { // przeszukujemy wektor dzieci w celu znalezienia szukanego name
		if (this->children_vector[i]->name == find_name) {
			return children_vector[i];  // zwracamy wskaźnik na PCB o znalezionym name
		}
	}

	for (int i = 0; i < this->children_vector.size(); i++) { // jeśli nie znajdziemy szukanego PID dla każdego z dzieci wywołujemy funkcję get_kid 
		children_vector[i]->get_kid_name(find_name); // rekurencja
	}

	return nullptr; // jeśli nie znajdziemy name , zwracamy nullptr
}

void PCB::kill_kid(int &kid_pid) {
	
	cout << "the process has been deleted !" << endl;
	for (int i = 0; i < this->children_vector.size(); i++) {
		if (children_vector[i]->pid == kid_pid) {
			//cout << children_vector[i]->name << " - " << children_vector[i]->pid << endl;
			this->children_vector.erase(this->children_vector.begin() + i); // usunięcie procesu 
			
		}
	}
	
	// przesunięcie pozostałych
}

void ProcTree::init(shared_ptr<PCB> init_proc) {
	this->init_proc = init_proc;
	memory.SetupInitProcess();
}

int ProcTree::create_process_file(string &name, string &file_name, int parent_pid)  // zakładająć że nie mamy podfolderów i ścieżka będzię jedynie nazwą pliku
{
	
	PCB_return = nullptr;
	find_bool = 0;
	shared_ptr<PCB> proc = ProcTree::find_name(this->init_proc, name);
	if (proc == nullptr) {

		//cout << parent_pid << " " << this->init_proc->pid << "\n";
		if (parent_pid == this->init_proc->pid) {
			proc = make_shared<PCB>(name, parent_pid);  // utworzenie wskaźnika na proces - wywołanie konstruktora procesu
			this->init_proc->children_vector.push_back(proc); // dodanie procesu jako dziecka procesu INIT
			memory.LoadProgram(file_name, proc->pid); // Załaduj kod programu do pamięci wirtualnej
			cout << "process created" << endl;
			proc->show_info();
		}
		else {
			PCB_return = nullptr;
			find_bool = 0;
			shared_ptr<PCB> parent = ProcTree::find_pid(this->init_proc, parent_pid);
			//this->init_proc->get_kid_pid(parent_pid);
			if (find_bool == 1) {
				parent = PCB_return;
			}

			if (parent == nullptr) {
				cout << "No parent ! " << endl;
			}
			else {
				proc = make_shared<PCB>(name, parent_pid);  // utworzenie wskaźnika na proces - wywołanie konstruktora procesu
				parent->children_vector.push_back(proc); // dodanie procesu jako dziecka odnalezionego procesu rodzica
				memory.LoadProgram(file_name, proc->pid); // Załaduj kod programu do pamięci wirtualnej
				cout << "process created" << endl;
				proc->show_info();
			}
		}
		return proc->pid;
	}
	else {
		cout << "Process already exists !" << endl;
		return -1;
	}
}

shared_ptr<PCB> ProcTree::find_pid(shared_ptr<PCB> pcb_child, int &pid_proc)
{
	//cout << this->init_proc->name << " - " << this->init_proc->pid << endl;
	//PCB_return = nullptr;

	
	if (this->init_proc->pid == pid_proc) {
		return this->init_proc;
	}
	else {
		for (int i = 0; i < pcb_child->children_vector.size(); i++) {

			//cout<< pcb_child->children_vector[i]->name << " pid = "<< pcb_child->children_vector[i]->pid << endl;

			if (pcb_child->children_vector[i]->pid == pid_proc) {

				cout << "find pid = " << pcb_child->children_vector[i]->pid << endl;
				PCB_return =  pcb_child->children_vector[i];
				find_bool = 1;
				return PCB_return;
			}
			else {
				//
				//cout << "rekurencja " << endl;
				PCB_return = ProcTree::find_pid(pcb_child->children_vector[i], pid_proc);
			}
		}
		return PCB_return;
	}

	
}


shared_ptr<PCB> ProcTree::find_name(shared_ptr<PCB> pcb_child, string &name)
{
	//cout << this->init_proc->name << " - " << this->init_proc->pid << endl;
	//PCB_return = nullptr;


	if (this->init_proc->name == name) {
		return this->init_proc;
	}
	else {
		for (int i = 0; i < pcb_child->children_vector.size(); i++) {

			//cout<< pcb_child->children_vector[i]->name << " pid = "<< pcb_child->children_vector[i]->pid << endl;

			if (pcb_child->children_vector[i]->name == name) {

				cout << "find name = " << pcb_child->children_vector[i]->name << endl;
				PCB_return = pcb_child->children_vector[i];
				find_bool = 1;
				return PCB_return;
			}
			else {
				//
				//cout << "rekurencja " << endl;
				PCB_return = ProcTree::find_name(pcb_child->children_vector[i], name);
			}
		}
		return PCB_return;
	}


}


/*
shared_ptr<PCB> ProcTree::find_name(string &name) {

	shared_ptr<PCB> PCB_return = nullptr;

	for (int i = 0; i < init_proc->children_vector.size(); i++) { // przeszukujemy wektor dzieci w celu znalezienia szukanego PID
		if (init_proc->children_vector[i]->name == name) {
			return init_proc->children_vector[i];  // zwracamy wskaźnik na PCB o znalezionym name
		}
	}

	for (int i = 0; i < init_proc->children_vector.size(); i++) { // jeśli nie znajdziemy szukanego name dla każdego z dzieci wywołujemy funkcję get_kid 
		PCB_return = init_proc->children_vector[i]->get_kid_name(name); // rekurencja
	}

	return PCB_return;

}*/

void ProcTree::kill_pid(int pid) { // zabicie procesu po PID

	shared_ptr<PCB> PCB_proc_kill = ProcTree::find_pid(this->init_proc, pid); 
	shared_ptr<PCB> PCB_parent = ProcTree::find_pid(this->init_proc, PCB_proc_kill->parent_pid);
	bool vector_child = 0;
	process_state PCB_new_state;

	if (PCB_proc_kill == nullptr) {
		// niedobrze
		cout << "There are no processes with the PID provided" << endl;
	}
	else if (pid == 0) {
		cout << "unable to delete INIT process!" << endl;
	}
	else {
		vector_child = PCB_proc_kill->null_vector_child();
		if (vector_child == true) {
			// kill proces
			PCB_new_state = terminated;
			PCB_proc_kill->change_state(PCB_new_state);  // zmiana stanu na terminated 
			//przesłanie informacji do rodzica o zakończeniu się

			
			// wyświetlenie informacji o zakończonym procesie i jego statusie
			PCB_proc_kill->show_info();
			// usuwanie z drzewa
			PCB_parent->kill_kid(pid);


			//usuwanie z kolejek - czeka na zmiany w planiście
//Planist::remove_pcb_from_ready(pid); 
//Planist::remove_pcb_from_wait(pid); 

			if (PCB_parent->bool_kill == true) {
				cout << "parent kill " << endl;
				ProcTree::kill_pid(PCB_parent->pid);
			}

		}
		else {
			cout << "Process has children " << endl;
			PCB_new_state = wait;
			// planista - czeka na funkcję 
			// Planist::manage(); 
			PCB_proc_kill->change_state(PCB_new_state);
			PCB_proc_kill->bool_kill = 1;
			PCB_proc_kill->show_info();

		}

	}

}

void ProcTree::kill_name(string name) { // zabicie procesu po name

	shared_ptr<PCB> PCB_proc_kill = ProcTree::find_name(this->init_proc, name);
	shared_ptr<PCB> PCB_parent = ProcTree::find_pid(this->init_proc, PCB_proc_kill->parent_pid);
	bool vector_child = 0;
	process_state PCB_new_state;

	if (PCB_proc_kill == nullptr) {
		// niedobrze
		cout << "There are no processes with the NAME provided" << endl;
	}
	else if (PCB_parent->pid == 0) {
		cout << "unable to delete INIT process!" << endl;
	}
	else {
		vector_child = PCB_proc_kill->null_vector_child();
		if (vector_child == true) {
			// kill proces
			PCB_new_state = terminated;
			PCB_proc_kill->change_state(PCB_new_state);  // zmiana stanu na terminated 

			PCB_proc_kill->show_info();
			// usuwanie z drzewa
			PCB_parent->kill_kid(PCB_proc_kill->pid);

			//usuwanie z kolejek - czeka na zmiany w planiście
//Planist::remove_pcb_from_ready(pid); 
//Planist::remove_pcb_from_wait(pid); 

		}
		else {
			cout << "Process has children " << endl;
			PCB_new_state = wait;
			// planista - czeka na funkcję 
			// Planist::manage(); 
			PCB_proc_kill->change_state(PCB_new_state);
			PCB_proc_kill->show_info();

		}

	}

}

void ProcTree::show_vector_child(shared_ptr<PCB> proc_show) {

	for (int i = 0; i < proc_show->children_vector.size(); i++) {
	
		ProcTree::show_vector_child(proc_show->children_vector[i]);
		cout << proc_show->children_vector[i]->name << " - "<< proc_show->children_vector[i]->pid << " parent - " << proc_show->children_vector[i]->parent_pid << endl;

	}


}

void ProcTree::display_tree()
{
	cout << this->init_proc->name << " - "<< this->init_proc->pid << endl;

	ProcTree::show_vector_child(this->init_proc);

    // sposób wyświetlania drzewa do ugadania z Anią :) 
}
