package de.rub.nds.mlbbgenerator;

import de.rub.nds.modifiablevariable.util.ArrayConverter;
import de.rub.nds.tlsattacker.attacks.config.BleichenbacherCommandConfig;
import de.rub.nds.tlsattacker.attacks.pkcs1.BleichenbacherWorkflowGenerator;
import de.rub.nds.tlsattacker.attacks.pkcs1.BleichenbacherWorkflowType;
import de.rub.nds.tlsattacker.attacks.pkcs1.Pkcs1Vector;
import de.rub.nds.tlsattacker.attacks.pkcs1.Pkcs1VectorGenerator;
import de.rub.nds.tlsattacker.core.config.Config;
import de.rub.nds.tlsattacker.core.constants.RunningModeType;
import de.rub.nds.tlsattacker.core.protocol.message.ChangeCipherSpecMessage;
import de.rub.nds.tlsattacker.core.state.State;
import de.rub.nds.tlsattacker.core.util.CertificateFetcher;
import de.rub.nds.tlsattacker.core.workflow.DefaultWorkflowExecutor;
import de.rub.nds.tlsattacker.core.workflow.WorkflowExecutor;
import de.rub.nds.tlsattacker.core.workflow.WorkflowTrace;
import de.rub.nds.tlsattacker.core.workflow.action.SendAction;
import de.rub.nds.tlsattacker.core.workflow.factory.WorkflowConfigurationFactory;
import de.rub.nds.tlsattacker.core.workflow.factory.WorkflowTraceType;
import java.security.PublicKey;
import java.security.interfaces.RSAPublicKey;
import java.util.List;
import java.util.Random;
import java.io.File;  // Import the File class
import java.io.FileWriter;  // Import the FileWriter class
import java.io.IOException;  // Import the IOException class to handle errors

/*
 * A class which makes handshakes and send bleichenbacher vectors at random
 *
 * @author ic0ns
 */
public class BleichenbacherGenerator {

    private GeneratorConfig generatorConfig;

    public BleichenbacherGenerator(GeneratorConfig generatorConfig) {
        this.generatorConfig = generatorConfig;
    }

    public void start() {
        PublicKey publicKey = CertificateFetcher.fetchServerPublicKey(generatorConfig.createConfig());
        if (publicKey == null) {
            throw new RuntimeException("Could not fetch publicKey - there is probbably something wrong with the config?");
        }
        if (!(publicKey instanceof RSAPublicKey)) {
            throw new RuntimeException("The received public key is not an RSA publicKey?");
        }
        Config config = generatorConfig.createConfig();
        //config.setWorkflowExecutorShouldClose(false);

        // Here you can define the vectors you want to create - if you want different vectors you can write your own pcksVector generator
        List<Pkcs1Vector> vectors = Pkcs1VectorGenerator.generatePkcs1Vectors((RSAPublicKey) publicKey, generatorConfig.getManipulations(), config.getHighestProtocolVersion());
        Random r = new Random();

        FileWriter csvWriter;
        try {
            csvWriter = new FileWriter(generatorConfig.getOutputfolder() + File.separator + "Client Requests.csv");
            csvWriter.write("client_hello_random,label,skipped_ccs_fin" + System.lineSeparator());
        } catch (IOException e) {
            System.out.println("An error occurred when trying to write to the csv");
            e.printStackTrace();
            return;
        }

        for (int i = 0; i < generatorConfig.getIterations(); i++)//How many itereations do we want to do?
        {
            //choose a vector at random
            //ok since timing is of the table i just create the vectors life - if you want to have timing you need to randomize before you prepare
            //and be more careful in generally here
            Pkcs1Vector vector = vectors.get(r.nextInt(vectors.size() - 1));
            byte[] newRandom = new byte[32];
            r.nextBytes(newRandom);
            config.setDefaultClientRandom(newRandom);
            config.setUseFreshRandom(false);

            boolean skipWorkflow = false;
            BleichenbacherWorkflowType workflowType = BleichenbacherWorkflowType.CKE_CCS_FIN;
            String workflowString = "FALSE";

            if (generatorConfig.isSkip()){
                if(generatorConfig.isNoskip()){
                    // Flip a coin for the workflow we do, either CKE or CKE_CCS_FIN
                    if(r.nextBoolean()){
                        skipWorkflow = true;
                    }
                }else{
                    // We only need to do CKE
                    skipWorkflow = true;
                }
            }
            if(skipWorkflow) {
                workflowType = BleichenbacherWorkflowType.CKE;
                workflowString = "TRUE";
            }

            // Print infos about vector
            String helloBytesString = ArrayConverter.bytesToHexString(newRandom,false,false).toLowerCase().replace(" ", "").replaceAll("(?<=..)(..)", ":$1");
            try {
                csvWriter.write(helloBytesString + "," + vector.getName().replace(" ", "_").replace(",", "_") + "," + workflowString + System.lineSeparator());
            } catch (IOException e) {
                System.out.println("An error occurred when trying to write to the csv");
                e.printStackTrace();
                return;
            }
            System.out.println(vector.getName());

            // Execute workflow
            WorkflowTrace workflowTrace = BleichenbacherWorkflowGenerator.generateWorkflow(config, workflowType, vector.getEncryptedValue());
            State state = new State(config, workflowTrace);
            WorkflowExecutor executor = new DefaultWorkflowExecutor(state);
            executor.executeWorkflow();
            //we explicitly close the socket at the end of the handshake - this can potentially cause problem if the server is slower than the timeout

            try{
                // Do some busy waiting if necessary
                Thread.sleep(generatorConfig.getWaitingtime());
            } catch (InterruptedException ex) {
                Thread.currentThread().interrupt();
            }

        }
        try{
            csvWriter.close();
        } catch (IOException e) {
            System.out.println("An error occurred when trying to write to the csv");
            e.printStackTrace();
            return;
        }
    }

}
