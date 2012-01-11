//
// Agent.cpp
//

#include <signal.h>

#include <WipTime.h>
#include <WipLogger.h>
#include <OGMessage.h>
#include <M2MSession.h>
#include <UserApiException.h>
#include <WipProperties.h>
#include <M2MDeviceManager.h>

#include "Agent.h"

#define __AGENT_NAME__ "Concentrator"
#define __AGENT_MAJOR__ 1
#define __AGENT_MINOR__ 0
#define __AGENT_REVISION__ 0

#define __MODIFY_TEXT_COMMAND__ "ModifyText_c"

/**
 * <p>
 * <b>OpenGate M2M/Mobility Communications Platform</b>
 * </p>
 * <p>
 * <i>Wireless your bussiness</i>
 * </p>
 * <p>
 * If you want more info about this example please send an email to
 * <a href="mailto:support@amplia.es">Amplia Support</a>
 * <p>
 * Copyright 2007 Amplia Soluciones S.L. All rights reserved.
 *
 * @author Amplia Soluciones S.L
 */

volatile bool g_isRunning = true;

// Inner function  for handling SIGINT (CTRL+C) signal. If detected,
// this agent finishes.
void Control_C(int signal)
{
  g_isRunning = false;
}

Agent::Agent(WipReactor *_reactor, const WipString& _configPath, const WipString& _tempPath)
	: m_reactor(_reactor), m_configPath(_configPath), m_tempPath(_tempPath)
{
	// Retrieves the unique instance of the M2MSession singleton object.
	// @see M2MSession
	m_session = M2MSession::GetInstance(m_reactor);
	// Establish this agent as a listener to the OpenGateSession events.
	m_session->SetListener(this);

  ::signal(SIGINT,Control_C);
}

Agent::~Agent()
{
	// Release associated resources.
	M2MSession::FinalizeInstance();
}

void Agent::LoadConfiguration() /*throws WipPropertiesException*/
{
  // Establish the desired OpenGate connectors enabled for this agent.
  int selectedCarriers = M2MSession::C_GPRS;

#ifdef WIP_LOGGER_DEBUG
  bool enableAppendLog = false;
  bool enableLogConsole = true;
  int maxLogFileSize = 524288;
  int maxLogFileBackupIndex = 0;

	WipLogger::GetInstance().SetEnableSystemOut(enableLogConsole);
	WipLogger::GetInstance().SetEnableAppendLog(enableAppendLog);
  WipLogger::GetInstance().InitFile(maxLogFileSize,maxLogFileBackupIndex,m_tempPath+"/agent.log");
#endif

  // Performs basic configuration.
	// \param selectedCarriers Enabled connectors.
	// \param "ogm2m.cfg" Configuration file. Provided with this
	// example. See directory ./../conf/.
	this->Configure(selectedCarriers, m_configPath, "ogm2m.cfg");
}

void Agent::Initialize() /*throws WipException*/
{
	// Initializes agent action.
	this->Connect();
  M2MDeviceManager::GetInstance()->GetIOManager()->EnableSecondarySerialPort(true);
}

void Agent::Finalize() /*throws WipException*/
{
  this->Disconnect();
}

void Agent::Configure(const int carriers, const WipString &cfgFile, const WipString &cfgPath)
{
	m_session->Configure(carriers,cfgFile,cfgPath,__AGENT_NAME__,__AGENT_MAJOR__,__AGENT_MINOR__,__AGENT_REVISION__);
}

void Agent::Connect()
{
	m_session->Initialize();
	m_session->Connect();
}

bool Agent::IsConnected()
{
  return m_session->GetOpenGateSession()->IsConnected();
}

void Agent::Disconnect()
{
	m_session->Disconnect();
	m_session->Finalize();
}

bool Agent::IsRunning() const
{
	return g_isRunning;
}

bool Agent::HandleTimeout(const WipTimeValue &tv, void *arg)
{
  int carrier = M2MSession::C_GPRS;
  OGMessage msg;
  OGMessage::DataUnit *du;
  WIP_LOG_DEBUG("Agent::HandleTimeout()");

  // Setting the name of the message
  msg.SetOperationName("Example Event");

  // Adding data to the message
  du = new OGMessage::DataUnit();
  du->SetStringValue("Hello " + (m_text.Length()!=0?(WipString(m_text + " ") ):WipString("")) + "from OpenGate API");
  msg.AddData(du);

  // Sending an Event to an Application
  m_session->SendEvent(msg, carrier);

  return true;
}

void Agent::HandleResponse(const OGErrorConstants::ErrorCodes _errorCode, const WipString& _errorCodeStr, const OGMessage& _msg)
{
  WIP_LOG_INFO("Agent::HandleResponse() -> Respnse: " + _msg.ToString());
}

void Agent::HandleException(const OGErrorConstants::ErrorCodes _errorCode, const WipString& _errorCodeStr,
                                const OGMessage& _msg)
{
  WIP_LOG_INFO("Agent::HandleException() -> Error: " + _errorCodeStr + ", Message: " + _msg.ToString());
}

void Agent::HandleEvent(const OGMessage& _msg)
{
  WIP_LOG_INFO("Agent::HandleEvent() -> Event: " + _msg.ToString());
}

bool Agent::HandleCommand(const OGMessage& _cmd, OGMessage& _rsp)
{
  WIP_LOG_INFO("Agent::HandleCommand() -> Command: " + _cmd.ToString());

  if (_cmd.GetOperationName() == __MODIFY_TEXT_COMMAND__)
  {
    OGMessage::DataUnit *du = new OGMessage::DataUnit();

    m_text = (*_cmd.GetData().begin())->GetStringValue();

    du->SetStringValue("Message Modified OK");
    _rsp.AddData(du);

    return true;
  }
  return false;
}

void Agent::HandleNotification(const NotificationLevel _notificationType, const OGMessage& _msg, const OGErrorConstants::ErrorCodes _errorCode,
                        const WipString& _errorCodeStr)
{
  WIP_LOG_INFO("Agent::HandleNotification() -> Notification: " + _msg.ToString());
}

FileTransferEventAction Agent::HandleFileProgress(ManagedFileTransfer* transfer,
                                                      const ManagedFileTransfer::NotificationMask notification)
{
  WIP_LOG_INFO("Agent::HandleFileProgress()");
  return ACTION_OK;
}

void Agent::HandleLinkUp(const M2MSession::CarrierType carrier)
{
  WIP_LOG_INFO("*************************************************");
	WIP_LOG_INFO("Agent::HandleLinkUp() -> Carrier: " + WipString(carrier));
	WIP_LOG_INFO("*************************************************");

  if (carrier >= M2MSession::C_GPRS)
  {
    WIP_LOG_DEBUG("  Scheduling timer for sending Events every 10 seconds");
    this->m_timeoutId = m_reactor->ScheduleTimer(this, NULL, WipTimeValue(10, 0), WipTimeValue(10, 0));
  }
}

void Agent::HandleLinkDown(const M2MSession::CarrierType carrier)
{
  WIP_LOG_INFO("*************************************************");
	WIP_LOG_INFO("Agent::HandleLinkDown() - Carrier: " + WipString(carrier));
	WIP_LOG_INFO("*************************************************");

  if (carrier >= M2MSession::C_GPRS)
  {
    WIP_LOG_DEBUG("  Cancelling timer for sending Events");
    m_reactor->CancelTimer(this->m_timeoutId);
  }
}

void Agent::HandleOutgoingMessageStatus(unsigned int connectorId, IConnectorListener::OGMessageOutgoingStatus status, OGMessage &msg, IConnectorListener::OGMessageOutgoingReason reason)
{
  WIP_LOG_INFO("*************************************************");
  if (status == IConnectorListener::MESSAGE_SENT)
  {
    WIP_LOG_INFO("Agent::HandleOutgoingMessageStatus() - The Message has been sent.");
    WIP_LOG_INFO("    Connector number: " + WipString(connectorId));
  }
  else
  {
    WIP_LOG_INFO("Agent::HandleOutgoingMessageStatus() - The Message has been discarded");
    WIP_LOG_INFO("    Connector number: " + WipString(connectorId));
    WIP_LOG_INFO("    Reason number: " + WipString((int)reason));
  }
  WIP_LOG_INFO("*************************************************");
}

void Agent::HandleStartUpdate()
{
  WIP_LOG_INFO("Agent::HandleStartUpdate()");
}

void Agent::HandleEndUpdate(const OGErrorConstants::ErrorCodes errCode)
{
  WIP_LOG_INFO("Agent::HandleEndUpdate()");
}

void Agent::HandleSynchronizationSuccessful()
{
  WIP_LOG_INFO("Agent::HandleSynchronizationSuccessful()");
}

