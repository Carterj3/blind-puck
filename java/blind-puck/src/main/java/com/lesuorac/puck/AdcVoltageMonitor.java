package com.lesuorac.puck;

import java.io.BufferedWriter;
import java.nio.file.Files;
import java.nio.file.OpenOption;
import java.nio.file.Paths;
import java.nio.file.StandardOpenOption;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.cloud.netflix.feign.support.SpringMvcContract;
import org.springframework.scheduling.annotation.EnableAsync;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;
import org.springframework.stereotype.Component;

import feign.Feign;
import feign.jackson.JacksonDecoder;

@Component
@EnableAsync
@EnableScheduling
public class AdcVoltageMonitor {

	private Sparkfun sparkfun;

	@Autowired
	public AdcVoltageMonitor(Sparkfun sparkfun) {
		this.sparkfun = Feign.builder()//
				.contract(new SpringMvcContract())//
				.decoder(new JacksonDecoder())//
				.target(Sparkfun.class, "http://192.168.4.1");
	}

	@Scheduled(fixedRate = 15000)
	public void pollServer() {
		Long startTime = System.currentTimeMillis();
		Integer[] reads = new Integer[10];
		for (int i = 0; i < reads.length; i++) {
			reads[i] = sparkfun.getAdcVoltage();
		}
		Long stopTime = System.currentTimeMillis();

		// Files.newBufferedWriter() uses UTF-8 encoding by default
		try (BufferedWriter writer = Files.newBufferedWriter(Paths.get("log.txt"), StandardOpenOption.APPEND)) {
			StringBuilder sb = new StringBuilder();
			sb.append(String.format("%d, %d", startTime, stopTime));
			for(int i=0;i<reads.length;i++){
				sb.append(String.format(",%d", reads[i]));
			}
			sb.append(String.format("%n"));
			
			writer.append(sb.toString());
			
			System.out.print(String.format("Wrote data: [%s]", sb.toString()));
		} catch (Exception e) {
			e.printStackTrace();
		}
	}
}
