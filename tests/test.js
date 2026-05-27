const { Server } = require("../lib/index");

console.log("Testing ccemu-net JS wrapper...");

// Test 1: Check exports
console.log("Test 1: Module exports");
console.log("  Server:", typeof Server);

if (typeof Server !== "function") {
  console.error("FAIL: Server is not a constructor");
  process.exit(1);
}

// Test 2: Create server
console.log("Test 2: Server creation");
try {
  const server = new Server({
    port: 17778, // Use high port to avoid conflicts
    maxConnections: 10,
    workerThreads: 2,
  });

  console.log("  Server created successfully");
  console.log("  ports:", server.ports);
  console.log("  connections size:", server.connections.size);

  // Test event emitter methods
  server.on("connect", (client) => {
    console.log("Client connected:", client.id);
  });

  server.on("message", (data) => {
    console.log("Message received:", data);
  });

  // Shutdown after a short delay
  setTimeout(() => {
    console.log("Shutting down server...");
    server.shutdown();
    console.log("Server shut down successfully");
    console.log("\nAll tests passed!");
    process.exit(0);
  }, 1000);
} catch (e) {
  console.error("FAIL: Server creation failed:", e.message);
  console.error(e.stack);
  process.exit(1);
}
