//
// Agent.h
//

#ifndef _AGENT_H_
#define _AGENT_H_

#include <M2MSession.h>
#include <M2MSessionListener.h>
#include <WipEventHandler.h>
#include <WipHash.h>
#include <WipList.h>

class WipReactor;

/**
 * <p>
 * <b>OpenGate M2M/Mobility Communications Platform</b>
 * </p>
 * <p>
 * <i>Wireless your bussiness</i>
 * </p>
 * <p>
 * <b>Description:</b><br/>
 *  Connectivity basic example. Performs the basic configuration from a 
 * a specified configuration file with a similar format to that of java 
 * properties,
 *
 * </p>
 * <p>
 * If you want more info about this example please send an email to
 * <a href="mailto:support@amplia.es">Amplia Support</a>
 * <p>
 * Copyright 2007 Amplia Soluciones S.L. All rights reserved.
 *  	
 * @see OpenGateSessionListener Interface for listening to the events
 * notified by OpenGateSession, such as M2M Platform messages or
 * status change.
 *  The Opengate session will be active, and available from the first time
 * a configurated connector (anyone but offline) is available.
 *
 * @author Amplia Soluciones S.L
 */

class Agent : public M2MSessionListener, public WipEventHandler
{
public:
	/** Agent constructor
	 * @param reactor WipReactor instance. It is needed by the api internal logic.
	 * @see Example 7_Interaction
	 */
	Agent(WipReactor *reactor, const WipString& _configPath, const WipString& _tempPath);
	/** Default destructor. 
	 * Finalizes this instance and release associated resources.
	 */
	~Agent();

	void LoadConfiguration(); /*throws WipPropertiesException*/

  void Initialize(); /*throws WipException*/
  bool IsRunning() const;
  void Finalize(); /*throws WipException*/

  bool IsConnected();

  // Temporizacion
  bool HandleTimeout(const WipTimeValue &tv, void *arg = 0);
  WIP_HANDLE GetHandle(void) const {return -1;};
  bool Close(void) {return true;};

private:
  WipReactor *m_reactor;
	M2MSession *m_session;
  WipString m_configPath;
  WipString m_tempPath;

	/** Configure method. Performs the M2M platform Opengate basic configuration
	 * @param carriers Desired available connectors. ORed of the selected connectors.
	 * @param cfgFile URL of the configuration file. See the file "ogm2m" provided, in the <code>..\,,\conf</code> directory.
	 * @see M2MSession::Configure in the provided documentation.
	 * @see M2MSession to look for available connectors.
	 */
	void Configure(const int carriers, const WipString &cfgFile, const WipString &cfgPath);

	void Connect();
	void Disconnect();

	/**
	 * Opengate platform message notification callbacks.
	 * @see M2MSessionListener in the provided documentation.
	 */
	void HandleResponse(const OGErrorConstants::ErrorCodes _errorCode, const WipString& _errorCodeStr, const OGMessage& _msg);
  void HandleException(const OGErrorConstants::ErrorCodes _errorCode, const WipString& _errorCodeStr,
                                 const OGMessage& _msg);
  void HandleEvent(const OGMessage& _msg);
  bool HandleCommand(const OGMessage& _cmd, OGMessage& _rsp);
  void HandleNotification(const NotificationLevel _notificationType, const OGMessage& _msg, const OGErrorConstants::ErrorCodes _errorCode,
                                    const WipString& _errorCodeStr);
  FileTransferEventAction HandleFileProgress(ManagedFileTransfer* transfer,
                                                       const ManagedFileTransfer::NotificationMask notification);
  void HandleLinkUp(const M2MSession::CarrierType carrier);
  void HandleLinkDown(const M2MSession::CarrierType carrier);
  void HandleOutgoingMessageStatus(unsigned int connectorId, IConnectorListener::OGMessageOutgoingStatus status, OGMessage &msg, IConnectorListener::OGMessageOutgoingReason reason);
  void HandleStartUpdate();
  void HandleEndUpdate(const OGErrorConstants::ErrorCodes errCode);
  void HandleSynchronizationSuccessful();

  // Sending Event timeout id
  unsigned long m_timeoutId;
  // For Sending in the Event
  WipString m_text;

};

#endif // _AGENT_H_
