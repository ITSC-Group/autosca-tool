/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */
package de.rub.nds.mlbbgenerator;

import com.beust.jcommander.JCommander;
import com.beust.jcommander.ParameterException;
import de.rub.nds.tlsattacker.core.config.delegate.GeneralDelegate;
import de.rub.nds.tlsattacker.core.exceptions.ConfigurationException;
import java.security.Security;
import org.apache.logging.log4j.LogManager;
import org.apache.logging.log4j.Logger;
import org.bouncycastle.jce.provider.BouncyCastleProvider;

public class Main {

    private static final Logger LOGGER = LogManager.getLogger();
    
    public static void main(String args[]) {
        Security.addProvider(new BouncyCastleProvider());

        GeneratorConfig config = new GeneratorConfig(new GeneralDelegate());
        JCommander commander = new JCommander(config);
        try {
            commander.parse(args);
            if (config.getGeneralDelegate().isHelp()) {
                commander.usage();
                return;
            }
            
            try {
                BleichenbacherGenerator generator = new BleichenbacherGenerator(config);
                generator.start();
            } catch (ConfigurationException E) {
                LOGGER.warn("Encountered a ConfigurationException aborting. Try -debug for more info");
                //LOGGER.debug(E);
                commander.usage();
            }
        } catch (ParameterException E) {
            LOGGER.warn("Could not parse provided parameters. Try -debug for more info");
            //LOGGER.debug(E);
            commander.usage();
        }
    }
}
