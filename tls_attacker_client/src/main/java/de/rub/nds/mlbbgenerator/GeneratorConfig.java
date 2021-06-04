/*
 * TLS-Attacker - A Modular Penetration Testing Framework for TLS
 *
 * Copyright 2014-2020 Ruhr University Bochum, Paderborn University,
 * and Hackmanit GmbH
 *
 * Licensed under Apache License 2.0
 * http://www.apache.org/licenses/LICENSE-2.0
 */
package de.rub.nds.mlbbgenerator;

import com.beust.jcommander.Parameter;
import com.beust.jcommander.ParametersDelegate;
import de.rub.nds.tlsattacker.attacks.config.BleichenbacherCommandConfig;
import de.rub.nds.tlsattacker.attacks.pkcs1.BleichenbacherWorkflowType;
import de.rub.nds.tlsattacker.core.config.Config;
import de.rub.nds.tlsattacker.core.config.TLSDelegateConfig;
import de.rub.nds.tlsattacker.core.config.delegate.CiphersuiteDelegate;
import de.rub.nds.tlsattacker.core.config.delegate.ClientDelegate;
import de.rub.nds.tlsattacker.core.config.delegate.GeneralDelegate;
import de.rub.nds.tlsattacker.core.config.delegate.ProtocolVersionDelegate;
import de.rub.nds.tlsattacker.core.config.delegate.StarttlsDelegate;
import de.rub.nds.tlsattacker.core.constants.AlgorithmResolver;
import de.rub.nds.tlsattacker.core.constants.CipherSuite;
import de.rub.nds.tlsattacker.core.constants.KeyExchangeAlgorithm;
import java.util.LinkedList;
import java.util.List;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;

/**
 *
 */
public class GeneratorConfig extends TLSDelegateConfig {

    private Logger LOGGER = LogManager.getLogger();

    @ParametersDelegate
    private ClientDelegate clientDelegate;

    @ParametersDelegate
    private CiphersuiteDelegate ciphersuiteDelegate;

    @ParametersDelegate
    private ProtocolVersionDelegate protocolVersionDelegate;

    @ParametersDelegate
    private StarttlsDelegate starttlsDelegate;

    @Parameter(names = "--workflow", description = "Which workflow traces should be tested with. Possible values: CKE_CCS_FIN, CKE, CKE_CCS, CKE_FIN")
    private BleichenbacherWorkflowType workflowType = BleichenbacherWorkflowType.CKE_CCS_FIN;

    @Parameter(names = "--repetitions", description = "The number of handshakes the client should perform")
    private int repetitions = 10000;

    @Parameter(names = "--folder", description = "The folder where the csv with the executed requests should be written to")
    private String outputfolder = "";

    @Parameter(names = "--skip", description = "Whether to use the workflow CKE")
    private boolean skip = false;

    @Parameter(names = "--noskip", description = "Whether to use the workflow CKE_CCS_FIN")
    private boolean noskip = false;

    @Parameter(names = "--manipulations", description = "Which padding manipulation to use")
    private BleichenbacherCommandConfig.Type manipulations = BleichenbacherCommandConfig.Type.FAST;

    @Parameter(names = "--timeout", description = "The number of milliseconds to wait for any response from the server")
    private int timeout = 50;

    @Parameter(names = "--clientauth", description = "Use client authentication")
    private boolean clientauth = false;

    @Parameter(names = "--sni", description = "Use the server name indicator extension")
    private boolean sni = false;

    @Parameter(names = "--wait", description = "The number of milliseconds to wait idly between requests")
    private int waitingtime = 0;

    @Parameter(names = "--twoclass", description = "Instead of randomly choosing between all vectors, choose only between correct padding and wrong version number")
    private boolean twoclass = 0;


    /**
     *
     * @param delegate
     */
    public GeneratorConfig(GeneralDelegate delegate) {
        super(delegate);
        clientDelegate = new ClientDelegate();
        ciphersuiteDelegate = new CiphersuiteDelegate();
        protocolVersionDelegate = new ProtocolVersionDelegate();
        starttlsDelegate = new StarttlsDelegate();
        addDelegate(clientDelegate);
        addDelegate(ciphersuiteDelegate);
        addDelegate(protocolVersionDelegate);
        addDelegate(starttlsDelegate);
    }

    /**
     *
     * @return
     */
    @Override
    public Config createConfig() {
        Config config = super.createConfig();
        if (ciphersuiteDelegate.getCipherSuites() == null) {
            List<CipherSuite> cipherSuites = new LinkedList<>();
            for (CipherSuite suite : CipherSuite.getImplemented()) {
                if (AlgorithmResolver.getKeyExchangeAlgorithm(suite) == KeyExchangeAlgorithm.RSA
                        || AlgorithmResolver.getKeyExchangeAlgorithm(suite) == KeyExchangeAlgorithm.PSK_RSA) {
                    cipherSuites.add(suite);
                }
            }
            config.setDefaultClientSupportedCiphersuites(cipherSuites);
        } else {
            for (CipherSuite suite : ciphersuiteDelegate.getCipherSuites()) {
                if (!suite.name().contains("RSA")) {
                    LOGGER.warn("CipherSuite list contains non RSA cipher suites - this may fail!");
                }
            }
        }
        config.setQuickReceive(true);
        config.setEarlyStop(true);
        config.setStopActionsAfterIOException(true);
        config.setStopActionsAfterFatal(true);
        config.setStopReceivingAfterFatal(true);
        config.setStopTraceAfterUnexpected(true);
        
        config.setAddRenegotiationInfoExtension(true);
        config.setAddServerNameIndicationExtension(sni);
        config.setAddSignatureAndHashAlgorithmsExtension(true);
        config.setAddECPointFormatExtension(false);
        config.setAddEllipticCurveExtension(false);
        config.getDefaultClientConnection().setTimeout(timeout);

        if(clientauth){
            config.setClientAuthentication(true);
        }

        return config;
    }

    public int getIterations() {
        return repetitions;
    }

    public void setIterations(int repetitions) {
        this.repetitions = repetitions;
    }

    public BleichenbacherWorkflowType getWorkflowType() {
        return workflowType;
    }

    public void setWorkflowType(BleichenbacherWorkflowType workflowType) {
        this.workflowType = workflowType;
    }

    public BleichenbacherCommandConfig.Type getManipulations() {
        return manipulations;
    }

    public void setManipulations(BleichenbacherCommandConfig.Type manipulations) {
        this.manipulations = manipulations;
    }

    public String getOutputfolder() {
        return outputfolder;
    }

    public void setOutputfolder(String outputfolder) {
        this.outputfolder = outputfolder;
    }

    public boolean isSkip() {
        return skip;
    }

    public void setSkip(boolean skip) {
        this.skip = skip;
    }

    public boolean isNoskip() {
        return noskip;
    }

    public void setNoskip(boolean noskip) {
        this.noskip = noskip;
    }

    public int getWaitingtime() {
        return waitingtime;
    }

    public void setWaitingtime(int waitingtime) {
        this.waitingtime = waitingtime;
    }

    public boolean isTwoclass() {
        return twoclass;
    }

    public void setTwoclass(boolean twoclass) {
        this.twoclass = twoclass;
    }

}
