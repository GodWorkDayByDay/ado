#pragma once

namespace ADO_NS
{
	struct CADO_IMPL
	{
		CADO_IMPL(CONFIG& cfg) : config_(cfg){}

		_ConnectionPtr m_pConnection_;
		_RecordsetPtr m_pRecordset_;
		_CommandPtr m_pCommand_;

		CONFIG config_;

		tstring m_tstrQuery_;
		bool is_first_record_;
	};

	template<typename T>
	void CADO::GetFieldValue(IN TCHAR* tszName, OUT T& Value)
	{
		_variant_t& vFieldValue = pImpl->m_pRecordset_->GetCollect(tszName);

		switch (vFieldValue.vt)
		{
		case VT_BOOL:		//bool
		case VT_I1:			//BYTE WORD
		case VT_I2:
		case VT_UI1:
		case VT_I4:			//DWORD
		case VT_DECIMAL:	//INT64
		case VT_R8:			//float double
		case VT_DATE:
			Value = vFieldValue;
			break;
		case VT_NULL:
		case VT_EMPTY:
			assert(0);
			return;
		default:
			assert(0);
			return;
		}
	}


	template<typename T> struct DataType{};
	template<>	struct DataType<bool> { const static DataTypeEnum value{ adBoolean }; };
	template<>	struct DataType<char> { const static DataTypeEnum value{ adTinyInt }; };
	template<>	struct DataType<unsigned char> { const static DataTypeEnum value{ adTinyInt }; };
	template<>	struct DataType<short> { const static DataTypeEnum value{ adSmallInt }; };
	template<>	struct DataType<unsigned short> { const static DataTypeEnum value{ adSmallInt }; };
	template<>	struct DataType<int> { const static DataTypeEnum value{ adInteger }; };
	template<>	struct DataType<unsigned int> { const static DataTypeEnum value{ adInteger }; };
	template<>	struct DataType<__int64> { const static DataTypeEnum value{ adBigInt }; };
	template<>	struct DataType<unsigned __int64> { const static DataTypeEnum value{ adBigInt }; };
	template<>	struct DataType<float> { const static DataTypeEnum value{ adDouble }; };
	template<>	struct DataType<ATL::COleDateTime> { const static DataTypeEnum value{ adDBTimeStamp }; };
	template<>	struct DataType<_variant_t>	{};


	template<typename T>
	void CADO::CreateParameter(IN TCHAR* tszName, IN enum ParameterDirectionEnum Direction, IN T& rValue)
	{
		_ParameterPtr pParametor = pImpl->m_pCommand_->CreateParameter(tszName, DataType<T>::value, Direction, sizeof(rValue));
		pImpl->m_pCommand_->Parameters->Append(pParametor);
		pParametor->Value = static_cast<_variant_t>(rValue);
	}

	template<typename T>
	void CADO::GetParameter(IN TCHAR* tszName, OUT T& Value)
	{
		_variant_t& vFieldValue = pImpl->m_pCommand_->Parameters->GetItem(tszName)->Value;

		switch (vFieldValue.vt)
		{
		case VT_BOOL:	//bool
		case VT_I1:
		case VT_I2:		//BYTE WORD
		case VT_UI1:
		case VT_I4:		//DWORD
		case VT_DECIMAL: //INT64
		case VT_R8:	//float double
		case VT_DATE:
			Value = vFieldValue;
			break;
		case VT_NULL:
		case VT_EMPTY:
			assert(0);
			return;
		default:
			assert(0);
			return;
		}
	}

	void dump_com_error(_com_error &e)
	{
		_tprintf(_T("Code = %08lX   Code meaning = %s\n"), e.Error(), e.ErrorMessage());
		_tprintf(_T("Source = %s\n"), (_TCHAR*)e.Source());
		_tprintf(_T("Desc = %s\n"), (_TCHAR*)e.Description());
	}
}

