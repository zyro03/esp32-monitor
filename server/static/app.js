function updateClock() {
  const clock = document.getElementById("clock");

  if (clock) {
    clock.textContent = new Date().toLocaleTimeString("pl-PL");
  }
}

updateClock();
setInterval(updateClock, 1000);

const chartElement = document.getElementById("measurementChart");

if (chartElement) {
  fetch("/chart-data")
    .then(function (response) {
      return response.json();
    })
    .then(function (data) {
      new Chart(chartElement, {
        type: "line",
        data: {
          labels: data.labels,
          datasets: [
            {
              label: "Temperatura [°C]",
              data: data.temperatures,
              borderWidth: 2,
            },
            {
              label: "Wilgotność [%]",
              data: data.humidities,
              borderWidth: 2,
            },
          ],
        },
        options: {
          responsive: true,
          maintainAspectRatio: false,
        },
      });
    });
}
setInterval(function () {
  const active = document.activeElement;

  if (active.tagName !== "INPUT") {
    window.location.reload();
  }
}, 15000);
