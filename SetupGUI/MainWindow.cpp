#include "MainWindow.h"

using namespace System;
using namespace System::Windows::Forms;

[STAThreadAttribute]
void Main(array<String^>^ args) {
	Application::EnableVisualStyles();
	Application::SetCompatibleTextRenderingDefault(false);
	SetupGUI::MainWindow form;
	Application::Run(%form);
}

namespace SetupGUI {

	System::Void MainWindow::button1_Click(System::Object^  sender, System::EventArgs^  e) {
		CpuMonitor cpuMonitor;
		System::Threading::Thread::Sleep(100);
		int usagePerc = int(cpuMonitor.GetTotalUsage() + 0.5);
		cpuLoadLabel->Text = String::Format("{0}%", usagePerc);
	}

}
