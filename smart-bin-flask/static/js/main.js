// Initialize Chart.js
const ctx = document.getElementById('liveChart').getContext('2d');
const liveChart = new Chart(ctx, {
    type: 'line',
    data: {
        labels: [], // Time points
        datasets: [{
            label: 'Fill Level (%)',
            data: [],
            borderColor: '#0d6efd',
            tension: 0.4,
            fill: true,
            backgroundColor: 'rgba(13, 110, 253, 0.1)'
        }]
    },
    options: {
        responsive: true,
        scales: {
            y: { beginAtZero: true, max: 100 }
        },
        animation: false // Disable animation for smoother updates
    }
});

// Function to fetch data from the Flask API
async function fetchData() {
    try {
        const response = await fetch('/api/data');
        const data = await response.json();
        
        // Update Text Elements
        document.getElementById('level-display').innerText = data.level + '%';
        document.getElementById('humidity-display').innerText = data.humidity + '%';
        document.getElementById('temp-display').innerText = data.temperature + '°C'; // New Temp Field
        document.getElementById('last-updated').innerText = 'Last Updated: ' + data.last_updated;
        
        // Update Status Badge
        const statusBadge = document.getElementById('bin-status');
        statusBadge.innerText = data.status;
        if(data.status.includes('FULL')) {
            statusBadge.classList.replace('bg-secondary', 'bg-danger');
        } else {
            statusBadge.classList.replace('bg-danger', 'bg-secondary');
        }

        // Update Progress Bar
        const bar = document.getElementById('level-bar');
        bar.style.width = data.level + '%';
        if(data.level > 80) bar.classList.replace('bg-primary', 'bg-danger');
        else bar.classList.replace('bg-danger', 'bg-primary');

        // Update Lid Status UI
        const lidCard = document.getElementById('lid-card');
        const lidStatus = document.getElementById('lid-status');
        const motionBadge = document.getElementById('motion-badge');

        if (data.person_nearby) {
            lidCard.className = 'card shadow-sm h-100 lid-open';
            lidStatus.innerText = "OPEN";
            lidStatus.classList.add('text-success');
            motionBadge.className = 'badge bg-success mt-2';
            motionBadge.innerText = "Motion Detected";
        } else {
            lidCard.className = 'card shadow-sm h-100 lid-closed';
            lidStatus.innerText = "CLOSED";
            lidStatus.classList.remove('text-success');
            motionBadge.className = 'badge bg-secondary mt-2';
            motionBadge.innerText = "No Motion";
        }

        // Update Chart
        const timeNow = new Date().toLocaleTimeString();
        if (liveChart.data.labels.length > 20) {
            liveChart.data.labels.shift();
            liveChart.data.datasets[0].data.shift();
        }
        liveChart.data.labels.push(timeNow);
        liveChart.data.datasets[0].data.push(data.level);
        liveChart.update();

    } catch (error) {
        console.error("Error fetching data:", error);
    }
}

// Function to toggle simulation (calls the API)
async function toggleSimulation() {
    await fetch('/api/simulate-person', { method: 'POST' });
    fetchData(); // Update immediately
}

// Poll the server every 2 seconds
setInterval(fetchData, 2000);

// Initial call
fetchData();
