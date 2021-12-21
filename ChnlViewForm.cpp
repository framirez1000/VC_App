#include "pch.h"
#include "ChnlViewForm.h"

Double ListTest_CLI_Project::ChnlViewForm::GetFormulaEvaluation(String^ formula)
{
	String^ str = gcnew String("");
	int index = 0;
	for each (String ^ chnlName in m_pChnlViewNames) {
		if (formula->Contains(chnlName)) {
			str = System::Convert::ToString(ChnlsViewList[index / 2]->vValue);
			formula = formula->Replace(chnlName + "_U", str);
			str = System::Convert::ToString(ChnlsViewList[index / 2]->vSetPoint);
			formula = formula->Replace(chnlName, str);
		}
		index++;
	}
	Expression^ e = gcnew Expression(formula);
	Double val = e->calculate();
	val = Math::Round(val, 2);
	//Console::WriteLine("Expression eval: {0}", val);
	
	
	return Double(val);
}



System::Void ListTest_CLI_Project::ChnlViewForm::GetformEvalAtRamp(String^ formula)
{
	// Iterate NamesList to check if a scanned channel was included in formula
		// to make "formula" evaluation when channel ramping
	for (int i = 1; i < this->m_pChnlViewNames->Count; i+=2) {
		if (formula->Contains(this->m_pChnlViewNames[i])) {
			evaluateFormulaAtRamping = true;
			break;
		}
	}
	return System::Void();
}
