package com.lesuorac.puck;

import org.springframework.boot.SpringApplication;
import org.springframework.boot.autoconfigure.SpringBootApplication;
import org.springframework.cloud.netflix.feign.EnableFeignClients;
import org.springframework.cloud.netflix.feign.support.SpringMvcContract;

import feign.Feign;
import feign.jackson.JacksonDecoder;

@SpringBootApplication(scanBasePackageClasses = { Sparkfun.class })
@EnableFeignClients
public class BlindPuckApplication {

	public static void main(String[] args) {
		SpringApplication.run(BlindPuckApplication.class, args);
	}

	private Sparkfun sparkfun;

	public BlindPuckApplication() {
		sparkfun = Feign.builder()//
				.contract(new SpringMvcContract())//
				.decoder(new JacksonDecoder())//
				.target(Sparkfun.class, "http://192.168.4.1");
		
		
		String content = "";
		content += "3\n";
		content += "1.0 100 200 50 2200\n";
		content += "2.0 100 200 50 2200\n";
		content += "3.0 100 200 50 2200";
		
		sparkfun.configureSpiff("/puck.cfg", "w", content);
	}

}
