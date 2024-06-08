let temperatureLevel = document.getElementById('temperatureLevel');
let temperatureSensor = document.getElementById('temperatureSensor');
let humidityLevel = document.getElementById('humidityLevel');
let humiditySensor = document.getElementById('humiditySensor');
let soilLevel = document.getElementById('soilLevel');
let soilSensor = document.getElementById('soilSensor');
let flameLevel = document.getElementById('flameLevel');
let flameSensor = document.getElementById('flameSensor');
let smokeLevel = document.getElementById('smokeLevel');
let smokeSensor = document.getElementById('smokeSensor');
let btnLights = document.getElementById('btnLights');
let btnMode = document.getElementById('btnMode');
let btnPump = document.getElementById('btnPump');


window.addEventListener('load', () => {
    window.localStorage.clear();
    setInterval(receiveData, 1000);
});

btnLights.addEventListener("click", () => {
    sendReq("/lights")
});

btnMode.addEventListener("click", () => {
    sendReq("/mode")
});

btnPump.addEventListener("click", () => {
    sendReq("/pump")
});


function sendReq(url) {
    fetch(url)
        .then(resp => resp.json())
        .then((resp) => {
            console.log(resp);
            if (resp["status"] == "success")
                console.log("Pedido Realizado!");
            else
                alert(resp["message"]);
        })
        .catch(error => { console.error("## ERRO PEGANDO OS DADOS:" + error); })
}


function receiveData() {
    fetch('/dados')
        .then(resp => resp.json())
        .then((resp) => {
            console.log(resp);
            let dados = resp["data"].split('*');
            if (dados[0] == 'D') {

                temperatureSensor.innerHTML = dados[1] + "*C";
                if (dados[1] > 32)
                    temperatureLevel.innerHTML = "ALTA";
                else if(dados[1] >= 10)
                    temperatureLevel.innerHTML = "NORMAL";
                else
                    temperatureLevel.innerHTML = "BAIXA0";

                humiditySensor.innerHTML = dados[2] + "%";
                if (dados[2] >= 85)
                    humidityLevel.innerHTML = "ALTA";
                else if (dados[2] >= 45)
                    humidityLevel.innerHTML = "NORMAL";
                else
                    humidityLevel.innerHTML = "BAIXA";

                soilSensor.innerHTML = dados[3] + "%";
                if (dados[3] >= 85)
                    soilLevel.innerHTML = "HUMIDO";
                else if (dados[3] > 30)
                    soilLevel.innerHTML = "NORMAL";
                else
                    soilLevel.innerHTML = "SECO";


                flameSensor.innerHTML = dados[4] + "%";
                if (dados[4] >= 65) {
                    flameLevel.innerHTML = "FOGO DETECTADO";
                    document.getElementById('imgFlameLevel').src = "fire.png";
                    document.getElementById('imgFlameSensor').src = "fire.png";
                }
                else {
                    flameLevel.innerHTML = "SEM FOGO";
                    document.getElementById('imgFlameLevel').src = "nofire.png";
                    document.getElementById('imgFlameSensor').src = "nofire.png";
                }

                smokeSensor.innerHTML = dados[5] + "%";
                if (dados[5] >= 60) {
                    smokeLevel.innerHTML = "FUMO DETECTADO";
                    document.getElementById('imgSmokeLevel').src = "smoke.png";
                    document.getElementById('imgSmokeSensor').src = "smoke.png";
                }
                else {
                    smokeLevel.innerHTML = "SEM FUMO";
                    document.getElementById('imgSmokeLevel').src = "nosmoke.png";
                    document.getElementById('imgSmokeSensor').src = "nosmoke.png";
                }
                
                btnLights.innerHTML = (dados[6] == "1") ? "APAGAR" : "ACENDER";
                document.getElementById('imgLights').src = (dados[6] == "1") ? "lampOn.png" : "lampOff.png";
                
                btnPump.innerHTML = (dados[7] == "1") ? "DESLIGAR" : "LIGAR";
                document.getElementById('imgPump').src = (dados[7] == "1") ? "pumpOn.png" : "pumpOff.png";
                
                btnMode.innerHTML = (dados[8] == "A") ? "AUTOMÁTICO" : "MANUAL";
            }
        })
        .catch(error => { console.error("## ERRO PEGANDO OS DADOS:" + error); })
}
