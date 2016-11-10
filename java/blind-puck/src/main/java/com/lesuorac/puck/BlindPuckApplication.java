package com.lesuorac.puck;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.netflix.feign.EnableFeignClients;

@SpringBootApplication(scanBasePackageClasses = { Sparkfun.class, AdcVoltageMonitor.class })
@EnableFeignClients
public class BlindPuckApplication {

	public static void main(String[] args) {
		SpringApplication.run(BlindPuckApplication.class, args);
	}

	public BlindPuckApplication() {

	}

}
